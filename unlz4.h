#pragma once
#include <stdint.h>
#include <stdio.h>

struct unlz4_context_t {
  // file handles
  FILE* in;
  void *out;
  uint32_t out_pos;

  // modify input buffer size as you like ... for most use cases, bigger buffer aren't faster anymore - and even reducing to 1 byte works !
#define UNLZ4_READ_BUFFER_SIZE 16*1024
  unsigned char readBuffer[UNLZ4_READ_BUFFER_SIZE + 512]; // overrun protection?
  unsigned int  pos;
  unsigned int  available;
};

// decompress from file to memory
uint32_t unlz4_decompress(unlz4_context_t *ctx);
