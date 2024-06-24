// //////////////////////////////////////////////////////////
// smallz4cat.c
// Copyright (c) 2016-2019 Stephan Brumme. All rights reserved.
// see https://create.stephan-brumme.com/smallz4/
//
// "MIT License":
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software
// is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// This program is a shorter, more readable, albeit slower re-implementation of lz4cat ( https://github.com/Cyan4973/xxHash )

// compile: gcc smallz4cat.c -O3 -o smallz4cat -Wall -pedantic -std=c99 -s
// The static 8k binary was compiled using Clang and dietlibc (see https://www.fefe.de/dietlibc/ )

// Limitations:
// - skippable frames and legacy frames are not implemented (and most likely never will)
// - checksums are not verified (see https://create.stephan-brumme.com/xxhash/ for a simple implementation)

// Replace getByteFromIn() and sendToOut() by your own code if you need in-memory LZ4 decompression.
// Corrupted data causes a call to unlz4error().

// suppress warnings when compiled by Visual C++
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>  // stdin/stdout/stderr, fopen, ...
#include <stdlib.h> // exit()
#include <string.h> // memcpy

#include "unlz4.h"

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

/// error handler
static void unlz4error(const char* msg)
{
    fprintf(stderr, "%s\n", msg);
}


// ==================== I/O INTERFACE ====================


// read one byte from input, see getByteFromIn()  for a basic implementation
typedef unsigned char (*GET_BYTE)  (void* userPtr);
// write several bytes,      see sendBytesToOut() for a basic implementation
typedef void          (*SEND_BYTES)(const unsigned char*, unsigned int, void* userPtr);


// read multiple bytes from the source
#if 0
static int getBytes(lz4_context_t *ctx, void *p, uint32_t size) {
    
}
#endif

/// read a single byte (with simple buffering)
static unsigned char getByte(unlz4_context_t *ctx) // parameter "userPtr" not needed
{

  // refill buffer
  if (ctx->pos == ctx->available)
  {
    ctx->pos = 0;
    ctx->available = fread(ctx->readBuffer, 1, UNLZ4_READ_BUFFER_SIZE, ctx->in);
    if (ctx->available == 0)
      unlz4error("out of data");
  }

  // return a byte
  return ctx->readBuffer[ctx->pos++];
}

/// write a block of bytes to the buffer
static void sendBytes(unlz4_context_t *ctx, const unsigned char* data, unsigned int numBytes)
{
    memcpy((uint8_t*)ctx->out + ctx->out_pos, data, numBytes);
    ctx->out_pos += numBytes;
}

// ==================== LZ4 DECOMPRESSOR ====================

uint32_t unlz4_decompress(unlz4_context_t *ctx)
{
  ctx->out_pos = 0;
  ctx->pos = 0;
  ctx->available = 0;
  uint32_t out_size = 0;

  // signature
  unsigned char signature1 = getByte(ctx);
  unsigned char signature2 = getByte(ctx);
  unsigned char signature3 = getByte(ctx);
  unsigned char signature4 = getByte(ctx);
  unsigned int  signature  = (signature4 << 24) | (signature3 << 16) | (signature2 << 8) | signature1;
  unsigned char isModern   = (signature == 0x184D2204);
  unsigned char isLegacy   = (signature == 0x184C2102);
  if (!isModern && !isLegacy)
    unlz4error("invalid signature");

  unsigned char hasBlockChecksum   = FALSE;
  unsigned char hasContentSize     = FALSE;
  unsigned char hasContentChecksum = FALSE;
  unsigned char hasDictionaryID    = FALSE;
  if (isModern)
  {
    // flags
    unsigned char flags = getByte(ctx);
    hasBlockChecksum   = flags & 16;
    hasContentSize     = flags &  8;
    hasContentChecksum = flags &  4;
    hasDictionaryID    = flags &  1;

    // only version 1 file format
    unsigned char version = flags >> 6;
    if (version != 1) {
      unlz4error("only LZ4 file format version 1 supported");
      return -1;
    }

    // ignore blocksize
    getByte(ctx);

    // get content size
    if (hasContentSize) {
        out_size |= (unsigned int)(getByte(ctx));
        out_size |= (unsigned int)(getByte(ctx) << 8);
        out_size |= (unsigned int)(getByte(ctx) << 16);
        out_size |= (unsigned int)(getByte(ctx) << 24);
        // skip high 4 bytes of 64bit number
        getByte(ctx);
        getByte(ctx);
        getByte(ctx);
        getByte(ctx);
    } else {
      unlz4error("no content size in LZ4 header\n");
      return -1;
    }

    int numIgnore = 0;
    // ignore, skip 4 bytes
    if (hasDictionaryID)
      numIgnore += 4;

    // ignore header checksum (xxhash32 of everything up this point & 0xFF)
    numIgnore++;

    // skip all those ignored bytes
    while (numIgnore--)
      getByte(ctx);  // bruuuuuuuuuuuuuuuuuuh
  }

  // allocate buffer
  ctx->out = (void*)malloc(out_size + 1024);
  if (ctx->out == NULL) {
    unlz4error("memory allocation error\n");
    return -1;
  }

  // don't lower this value, backreferences can be 64kb far away
#define HISTORY_SIZE 64*1024
  // contains the latest decoded data
  unsigned char history[HISTORY_SIZE];
  // next free position in history[]
  unsigned int  pos = 0;

  // parse all blocks until blockSize == 0
  while (1)
  {
    // block size
    unsigned int blockSize = getByte(ctx);
    blockSize |= (unsigned int)getByte(ctx) <<  8;
    blockSize |= (unsigned int)getByte(ctx) << 16;
    blockSize |= (unsigned int)getByte(ctx) << 24;

    // highest bit set ?
    unsigned char isCompressed = isLegacy || (blockSize & 0x80000000) == 0;
    if (isModern)
      blockSize &= 0x7FFFFFFF;

    // stop after last block
    if (blockSize == 0)
      break;

    if (isCompressed)
    {
      // decompress block
      unsigned int blockOffset = 0;
      unsigned int numWritten  = 0;
      while (blockOffset < blockSize)
      {
        // get a token
        unsigned char token = getByte(ctx);
        blockOffset++;

        // determine number of literals
        unsigned int numLiterals = token >> 4;
        if (numLiterals == 15)
        {
          // number of literals length encoded in more than 1 byte
          unsigned char current;
          do
          {
            current = getByte(ctx);
            numLiterals += current;
            blockOffset++;
          } while (current == 255);
        }

        blockOffset += numLiterals;

        // copy all those literals
        if (pos + numLiterals < HISTORY_SIZE)
        {
          // fast loop
          while (numLiterals-- > 0)
            history[pos++] = getByte(ctx);
        }
        else
        {
          // slow loop
          while (numLiterals-- > 0)
          {
            history[pos++] = getByte(ctx);

            // flush output buffer
            if (pos == HISTORY_SIZE)
            {
              sendBytes(ctx, history, HISTORY_SIZE);
              numWritten += HISTORY_SIZE;
              pos = 0;
            }
          }
        }

        // last token has only literals
        if (blockOffset == blockSize)
          break;

        // match distance is encoded in two bytes (little endian)
        unsigned int delta = getByte(ctx);
        delta |= (unsigned int)getByte(ctx) << 8;
        // zero isn't allowed
        if (delta == 0)
          unlz4error("invalid offset");
        blockOffset += 2;

        // match length (always >= 4, therefore length is stored minus 4)
        unsigned int matchLength = 4 + (token & 0x0F);
        if (matchLength == 4 + 0x0F)
        {
          unsigned char current;
          do // match length encoded in more than 1 byte
          {
            current = getByte(ctx);
            matchLength += current;
            blockOffset++;
          } while (current == 255);
        }

        // copy match
        unsigned int referencePos = (pos >= delta) ? (pos - delta) : (HISTORY_SIZE + pos - delta);
        // start and end within the current 64k block ?
        if (pos + matchLength < HISTORY_SIZE && referencePos + matchLength < HISTORY_SIZE)
        {
          // read/write continuous block (no wrap-around at the end of history[])
          // fast copy
          if (pos >= referencePos + matchLength || referencePos >= pos + matchLength)
          {
            // non-overlapping
            memcpy(history + pos, history + referencePos, matchLength);
            pos += matchLength;
          }
          else
          {
            // overlapping, slower byte-wise copy
            while (matchLength-- > 0)
              history[pos++] = history[referencePos++];
          }
        }
        else
        {
          // either read or write wraps around at the end of history[]
          while (matchLength-- > 0)
          {
            // copy single byte
            history[pos++] = history[referencePos++];

            // cannot write anymore ? => wrap around
            if (pos == HISTORY_SIZE)
            {
              // flush output buffer
              sendBytes(ctx, history, HISTORY_SIZE);
              numWritten += HISTORY_SIZE;
              pos = 0;
            }
            // wrap-around of read location
            referencePos %= HISTORY_SIZE;
          }
        }
      }

      // all legacy blocks must be completely filled - except for the last one
      if (isLegacy && numWritten + pos < 8*1024*1024)
        break;
    }
    else
    {
      // copy uncompressed data and add to history, too (if next block is compressed and some matches refer to this block)
      while (blockSize-- > 0)
      {
        // copy a byte ...
        history[pos++] = getByte(ctx);
        // ... until buffer is full => send to output
        if (pos == HISTORY_SIZE)
        {
          sendBytes(ctx, history, HISTORY_SIZE);
          pos = 0;
        }
      }
    }

    if (hasBlockChecksum)
    {
      // ignore checksum, skip 4 bytes
      getByte(ctx); getByte(ctx); getByte(ctx); getByte(ctx);
    }
  }

  if (hasContentChecksum)
  {
    // ignore checksum, skip 4 bytes
    getByte(ctx); getByte(ctx); getByte(ctx); getByte(ctx);
  }

  // flush output buffer
  sendBytes(ctx, history, pos);

  return 0;     // we're done!
}
