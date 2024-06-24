/****
    custom memory allocators (linear\list\bitmap)
    --wbcbz7 l7.o6.zol8
    
    TODO: write proper exception classes!
***/

#ifndef __MEMORY_H_
#define __MEMORY_H_

#include <stdlib.h>
#include <stdio.h>
#include <exception>
#include <stdexcept>

class MemoryAllocator {
    
    private:
        // pointer to malloc'd memory - unaligned pool
        void        *unaligned_pool;
    
        // pointer to allocated pool
        void        *pool;
    
        // number of total blocks
        size_t      totalBlocks;
        
        // number of free blocks
        size_t      freeBlocks;

        // pool size
        size_t      poolSize;
    
    public:
        // create object
        MemoryAllocator();
        
        // init pool
        virtual void   init(size_t size);
        
        // allocate block of memory
        virtual void*  alloc(size_t size, size_t align = 0);
        
        // free block of memory
        virtual void   free(void *p);
        
        // return lagrest free block size
        virtual size_t largestFree();
        
        // print debug info
        virtual void   info(FILE *f = stdout);
        
        // delete pool
        ~MemoryAllocator();
};

class PoolAllocator : public MemoryAllocator {
    
    const size_t poolDefaultSize;
    
    // type consts
    enum BlockType {
        blockUsed    = 0x55AA55AA,
        blockFree    = 0x99669966
        // any other values are illegal and indicate chain corruption
    };
    
    // block header
    struct BlockEntry {
        BlockType   type;
        BlockEntry *next;       // NULL - last block in chain
    };
    
    private:
        // pointer to malloc'd memory - unaligned pool
        void        *unaligned_pool;
    
        // pointer to allocated pool
        void        *pool;
        
        // pointer to first BlockEntry
        BlockEntry  *first;
        
        // number of total blocks
        size_t      totalBlocks;
        
        // number of free blocks
        size_t      freeBlocks;

        // pool size
        size_t      poolSize;

        // insert new block
        virtual void*   insertBlock(BlockEntry *blk, size_t blockSize, size_t size, BlockType type);
        
        // find first fitting block in chain
        //BlockEntry *findFirstFit(BlockEntry *start, size_t size, size_t align);
        
        // find best fitting block in chain
        //void *      findBestFit(size_t size, size_t align);

        // concatenate free blocks
        void        concatenateFreeBlocks(BlockEntry *block = NULL);

    public:
        // create object
        PoolAllocator();

        // init pool
        virtual void  init(size_t size);
        
        // allocate block of memory
        virtual void* alloc(size_t size, size_t align = 0);
        
        // free block of memory
        virtual void  free(void *p);
        
        // return lagrest free block size
        virtual size_t largestFree();
        
        // print debug info
        virtual void  info(FILE *f = stdout);
        
        // delete pool
        ~PoolAllocator();

};

// bitmap memory allocator
class BitmapAllocator : public MemoryAllocator {
        
    const size_t poolDefaultSize;
    const size_t blockSize;
    
    // type consts
    enum BlockType {
        blockUsed   = 1,
        blockFree   = 0
    };
    
    private:
        // pointer to malloc'd memory - unaligned pool
        void            *unaligned_pool;
    
        // pointer to allocated pool
        void            *pool;
        
        // pointer to block bitmap
        unsigned char   *bitmap;
        
        // total blocks count
        size_t          totalBlocks;
        
        // pool size
        size_t          poolSize;

        // insert new block
        virtual void*   insertUsedBlock(size_t start, size_t size, unsigned char prev);
        
        // choose block "color"
        virtual unsigned char blockColor(unsigned char prev, unsigned char next);
        
        // find first fitting block in chain
        //BlockEntry *findFirstFit(BlockEntry *start, size_t size, size_t align);
        
        // find best fitting block in chain
        //void *      findBestFit(size_t size, size_t align);

    public:
        // create object
        BitmapAllocator();

        // init pool
        virtual void  init(size_t size);
        
        // allocate block of memory
        virtual void* alloc(size_t size, size_t align = 0);
        
        // free block of memory
        virtual void  free(void *p);
        
        // return lagrest free block size
        virtual size_t largestFree();
        
        // print debug info
        virtual void  info(FILE *f = stdout);
        
        // delete pool
        ~BitmapAllocator();

};

// simple linear allocator manager
class LinearAllocator : public MemoryAllocator {
    
    const size_t poolDefaultSize;

    
    private:
        // pointer to malloc'd memory - unaligned pool
        void        *unaligned_pool;
    
        // pointer to allocated pool
        void        *pool;
        
        // pointer to first free byte in pool
        void        *firstFree;
        
        // pool size
        size_t      poolSize;
        
    
    public:
        // create object
        LinearAllocator();

        // init pool
        virtual void  init(size_t size);
        
        // allocate block of memory
        virtual void* alloc(size_t size, size_t align = 0);
        
        // return lagrest free block size
        virtual size_t largestFree();
        
        // free block of memory - unapplicapable! :)
        //virtual void  free(void *p);
        
        // print debug info
        virtual void  info(FILE *f = stdout);
        
        // delete pool
        ~LinearAllocator();

};

// even simplier malloc()/free() interface
class StandardAllocator : public MemoryAllocator {
    
    public:
        // create object
        StandardAllocator();

        // init pool
        virtual void  init(size_t size);
        
        // allocate block of memory
        virtual void* alloc(size_t size, size_t align = 0);
        
        // return lagrest free block size
        virtual size_t largestFree();
        
        // free block of memory
        virtual void  free(void *p);
        
        // print debug info - absent
        //virtual void  info(FILE *f = stdout);
        
        // delete pool
        ~StandardAllocator();

};
#endif