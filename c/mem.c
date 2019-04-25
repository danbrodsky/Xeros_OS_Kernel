/* mem.c : memory manager
*/

#include <xeroskernel.h>
#include <i386.h>

/* Your code goes here */
int DEBUG_KMEMINIT = 1;
int DEBUG_KMALLOC = 1;
int DEBUG_KFREE = 0;
int TEST = 0;

// header for newly allocated memory
typedef struct MemBlock {
    size_t size; // size of the allocated memory
    struct MemBlock *prev; // previous allocated block
    struct MemBlock *next; // next allocated block
    long sanityCheck; // check to see if header not corrupted/overwritten
    unsigned char dataStart[0]; // label for beginning of data
} MemBlock;

size_t mem_round(size_t addr);
void merge_coincident(MemBlock *block);
void test_mem(void);
void print_free_list(void);

extern long freemem;
extern char *maxaddr;

MemBlock *freeMemStart;


//This function performs any initialization the memory manager requires.
void kmeminit( void ){
    if(DEBUG_KMEMINIT) {
        kprintf("free memory start: %x\n", mem_round(freemem));
        kprintf("free memory end: %x \n", maxaddr);
        kprintf("HOLE start: %x \n", HOLESTART);
        kprintf("HOLE end: %x \n", HOLEEND);
        kprintf("size of header %x \n", sizeof(MemBlock));
    }

    // init free mem block before hole
    freeMemStart = (MemBlock *) (mem_round(freemem));
    freeMemStart->size =  mem_round((size_t) HOLESTART - freemem);


    // init free mem block after hole
    MemBlock *afterhole =  (MemBlock *) (mem_round(HOLEEND));
    afterhole->size = mem_round((size_t) maxaddr - mem_round(HOLEEND));

    // adjust next,prev blocks
    afterhole->next = NULL;
    afterhole->prev = freeMemStart;
    freeMemStart->next = afterhole;
    freeMemStart->prev = NULL;

    if(TEST){
        test_mem();
    }
}


// rounds memory to order of 16 addresses
size_t mem_round(size_t addr){
    addr = (addr) / 16 + ((addr % 16) ? 1 : 0);
    return addr * 16;
}

// allocates memotry size + size(MemBlock) and returns the pointer to datastart
void *kmalloc( size_t size ){
    // if invalid size return
    if(size <= 0) {
        return NULL;
    }
    MemBlock *currBlock = freeMemStart;
    size = mem_round(size) + sizeof(MemBlock);
    while(currBlock != NULL){
        // check if free block size is large enough to allocate
        if(size <= currBlock->size){
            if(DEBUG_KMALLOC) kprintf( "free block found: %x \n", currBlock);
            size_t remainder = currBlock->size - size;
            if(DEBUG_KMALLOC) kprintf( "remainder: %d\n", remainder);
            if(!remainder){
                print_free_list();
                // remove whole block from free list if remainder is 0
                currBlock->sanityCheck = (long) currBlock;
                if(currBlock->prev){
                    currBlock->prev->next = currBlock->next;
                } else {
                    freeMemStart = currBlock->next;
                }
                if(currBlock->next){
                    currBlock->next->prev = currBlock->prev;
                }
                return currBlock->dataStart;
            } else{
                // split block and allocate "size" if remainder is not 0
                MemBlock *block = currBlock;
                block->size = size;
                block->sanityCheck = (long) currBlock;

                currBlock = (MemBlock *) (((size_t) block) + block->size);
                currBlock->size = remainder;
                currBlock->prev = block->prev;
                currBlock->next = block->next;

                if(currBlock->prev){
                    currBlock->prev->next = currBlock;
                } else {
                    freeMemStart = currBlock;
                }
                if(currBlock->next){
                    currBlock->next->prev = currBlock;
                }
                return  block->dataStart;
            }
        }
        currBlock = currBlock->next;
    }
    return NULL;
}

//implement the function kfree() that takes a pointer to a previously allocated chunk of
//memory and returns it to the free memory pool.
int kfree( void *ptr ){
    if(!ptr){
        return 0;
    }
    MemBlock* toFreeBlock = (MemBlock *) (ptr - sizeof(MemBlock));
    if(toFreeBlock->sanityCheck != (long) toFreeBlock){
        return 0;
    }
    // find adjacent block before the block to be freed
    MemBlock *adjNextBlock = freeMemStart;
    MemBlock *adjPrevBlock = NULL;
    while(adjNextBlock){
        if(toFreeBlock <= adjNextBlock){
            if(DEBUG_KFREE) kprintf( "to freeblock: %x \n", toFreeBlock);
            if(DEBUG_KFREE) kprintf( "adjNextBlock addr: %x \n", adjNextBlock);
            break;
        }
        adjPrevBlock = adjNextBlock;
        adjNextBlock = adjNextBlock->next;
    }

    // manipulate list
    toFreeBlock->next = adjNextBlock;
    toFreeBlock->prev = adjPrevBlock;
    toFreeBlock->sanityCheck = NULL;

    if(adjNextBlock){
        adjNextBlock->prev = toFreeBlock;
    }
    if(adjPrevBlock){
        adjPrevBlock->next = toFreeBlock;
    }
    if(adjNextBlock == freeMemStart){
        freeMemStart = toFreeBlock;
    }

    // merge if there exists a coincident block
    merge_coincident(toFreeBlock);
    return 1;
}

// merge coincident adjacent blocks
void merge_coincident(MemBlock *block){
    // check if prev block is coincident
    MemBlock *adjNextBlock = block->next;
    MemBlock *adjPrevBlock = block->prev;

    // if three blocks are all coincident merge
    if(adjNextBlock && adjPrevBlock && (size_t) adjPrevBlock + adjPrevBlock->size + block->size == (size_t) adjNextBlock){
        adjPrevBlock->size =  adjPrevBlock->size + adjNextBlock->size + block->size;
        adjPrevBlock->next = adjNextBlock->next;
        if(adjNextBlock->next){
            adjNextBlock->next->prev = adjPrevBlock;
        }
        return;
    }

    // if prev block is coincident merge
    if(adjPrevBlock && (size_t) adjPrevBlock + adjPrevBlock->size == (size_t)block){
        adjPrevBlock->size = adjPrevBlock->size + block->size;
        adjPrevBlock->next = adjNextBlock;
        if(adjNextBlock){
            adjNextBlock->prev = adjPrevBlock;
        }
    }

    // if next block is coincident merge
    if(adjNextBlock && (size_t) block + block->size == (size_t) block->next){
        block->size = block->size + block->next->size;
        block->next = block->next->next;
        if(block->next){
            block->next->prev = block;
        }
    }
}

// print all blocks in free list and its size
void print_free_list(void){
    MemBlock *freeblock = freeMemStart;
    int blockNum = 1;
    while(freeblock != NULL){
        kprintf("free mem %d: %x size:%d -> ", blockNum, freeblock, freeblock->size);
        blockNum ++;
        if(blockNum > 10) return;
        freeblock = freeblock->next;
    }
    kprintf("NULL \n ");
}

// test cases
void test_mem(void) {
    int i;

    char *block1 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n", block1); // should be first block before hole
    assert_equal((size_t)block1, mem_round((size_t) freemem + sizeof(MemBlock)));
    kprintf("------------------------------------\n");
    char *block2 = (char *) kmalloc(1010000);
    kprintf("kmalloc(1010000): %x\n", block2); // should be first block after hole
    print_free_list();
    kprintf("------------------------------------\n");

    char *block3 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n", block3); // should be a block before hole
    print_free_list();
    kprintf("------------------------------------\n");

    char *block4 = (char *) kmalloc(1010000);
    kprintf("kmalloc(1010000): %x\n", block4); // should be last block after hole
    print_free_list();
    kprintf("------------------------------------\n");

    char *block5 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n", block5); // should be a block before hole
    print_free_list();
    kprintf("------------------------------------\n");

    char *block6 = (char *) kmalloc(101000000);
    kprintf("kmalloc(101000000): %x\n", block6); // should return null
    print_free_list();
    kprintf("------------------------------------\n");


    kprintf("kfree(%x): %x\n", block1, kfree(block1)); // should return 1
    print_free_list();
    block1 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n\n", block1); // should be first block before hole
    kprintf("kfree(%x): %x\n", block2, kfree(block2)); // should return 1
    block2 = (char *) kmalloc(1010000);
    print_free_list();
    kprintf("kmalloc(1010000): %x\n", block2); // should be first block after hole
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block1, kfree(block1)); // should return 1
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block3, kfree(block2)); // should return 1
    print_free_list();
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block2, kfree(block3)); // should return 1
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block4, kfree(block4)); // should return 1
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block5, kfree(block5)); // should return 1
    kprintf("------------------------------------\n");
    print_free_list();

    for (i = 0; i < 3000000; i++);


    block1 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n", block1); // should be first block before hole
    kprintf("------------------------------------\n");
    block2 = (char *) kmalloc(1010000);
    kprintf("kmalloc(1010000): %x\n", block2); // should be first block after hole
    print_free_list();
    kprintf("------------------------------------\n");

    block3 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n", block3); // should be a block before hole
    print_free_list();
    kprintf("------------------------------------\n");

    block4 = (char *) kmalloc(1010000);
    kprintf("kmalloc(1010000): %x\n", block4); // should be last block after hole
    print_free_list();
    kprintf("------------------------------------\n");

    block5 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n", block5); // should be a block before hole
    print_free_list();
    kprintf("------------------------------------\n");

    block6 = (char *) kmalloc(101000000);
    kprintf("kmalloc(101000000): %x\n", block6); // should return null
    print_free_list();
    kprintf("------------------------------------\n");


    kprintf("kfree(%x): %x\n", block1, kfree(block1)); // should return 1
    print_free_list();
    block1 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n\n", block1); // should be first block before hole
    kprintf("kfree(%x): %x\n", block2, kfree(block2)); // should return 1
    block2 = (char *) kmalloc(1010000);
    print_free_list();
    kprintf("kmalloc(1010000): %x\n", block2); // should be first block after hole
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block1, kfree(block5)); // should return 1
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block3, kfree(block2)); // should return 1
    print_free_list();
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block2, kfree(block1)); // should return 1
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block4, kfree(block3)); // should return 1
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block5, kfree(block4)); // should return 1
    kprintf("------------------------------------\n");
    print_free_list();


    //  for (i = 0; i < 3000000; i++);


    block1 = (char *) kmalloc(1000);
    kprintf("block 1 kmalloc(1000): %x\n", block1); // should be first block before hole
    kprintf("------------------------------------\n");
    block2 = (char *) kmalloc(1010000);
    kprintf("block 2 kmalloc(1010000): %x\n", block2); // should be first block after hole
    print_free_list();
    kprintf("------------------------------------\n");

    block3 = (char *) kmalloc(1000);
    kprintf("block 3 kmalloc(1000): %x\n", block3); // should be a block before hole
    print_free_list();
    kprintf("------------------------------------\n");

    block4 = (char *) kmalloc(1010000);
    kprintf("block 4 kmalloc(1010000): %x\n", block4); // should be last block after hole
    print_free_list();
    kprintf("------------------------------------\n");

    block5 = (char *) kmalloc(1000);
    kprintf("block 5 kmalloc(1000): %x\n", block5); // should be a block before hole
    print_free_list();
    kprintf("------------------------------------\n");

    block6 = (char *) kmalloc(101000000);
    kprintf("kmalloc(101000000): %x\n", block6); // should return null
    print_free_list();
    kprintf("------------------------------------\n");

    kprintf("kfree(%x): %x\n", block1, kfree(block1)); // should return 1
    print_free_list();
    block1 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n\n", block1); // should be first block before hole
    kprintf("kfree(%x): %x\n", block2, kfree(block2)); // should return 1
    block2 = (char *) kmalloc(1010000);
    print_free_list();
    kprintf("kmalloc(1010000): %x\n", block2); // should be first block after hole
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block1, kfree(block4)); // should return 1
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block3, kfree(block1)); // should return 1
    print_free_list();
    kprintf("------------------------------------\n");
    kprintf("block2: %x \n", block5);
    kprintf("kfree(%x): %x\n", block5, kfree(block5)); // should return 1
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block4, kfree(block2)); // should return 1
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block5, kfree(block3)); // should return 1
    kprintf("------------------------------------\n");
    print_free_list();


    for (i = 0; i < 3000000; i++);


    block1 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n", block1); // should be first block before hole
    kprintf("------------------------------------\n");
    block2 = (char *) kmalloc(1010000);
    kprintf("kmalloc(1010000): %x\n", block2); // should be first block after hole
    print_free_list();
    kprintf("------------------------------------\n");

    block3 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n", block3); // should be a block before hole
    print_free_list();
    kprintf("------------------------------------\n");

    block4 = (char *) kmalloc(1010000);
    kprintf("kmalloc(1010000): %x\n", block4); // should be last block after hole
    print_free_list();
    kprintf("------------------------------------\n");

    block5 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n", block5); // should be a block before hole
    print_free_list();
    kprintf("------------------------------------\n");

    block6 = (char *) kmalloc(101000000);
    kprintf("kmalloc(101000000): %x\n", block6); // should return null
    print_free_list();
    kprintf("------------------------------------\n");


    kprintf("kfree(%x): %x\n", block1, kfree(block1)); // should return 1
    print_free_list();
    block1 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n\n", block1); // should be first block before hole
    kprintf("kfree(%x): %x\n", block2, kfree(block2)); // should return 1
    block2 = (char *) kmalloc(1010000);
    print_free_list();
    kprintf("kmalloc(1010000): %x\n", block2); // should be first block after hole
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block1, kfree(block3)); // should return 1
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block3, kfree(block1)); // should return 1
    print_free_list();
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block2, kfree(block4)); // should return 1
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block4, kfree(block5)); // should return 1
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block5, kfree(block2)); // should return 1
    kprintf("------------------------------------\n");
    print_free_list();


    for (i = 0; i < 3000000; i++);

    block1 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n", block1); // should be first block before hole
    kprintf("------------------------------------\n");
    block2 = (char *) kmalloc(1010000);
    kprintf("kmalloc(1010000): %x\n", block2); // should be first block after hole
    print_free_list();
    kprintf("------------------------------------\n");

    block3 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n", block3); // should be a block before hole
    print_free_list();
    kprintf("------------------------------------\n");

    block4 = (char *) kmalloc(1010000);
    kprintf("kmalloc(1010000): %x\n", block4); // should be last block after hole
    print_free_list();
    kprintf("------------------------------------\n");

    block5 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n", block5); // should be a block before hole
    print_free_list();
    kprintf("------------------------------------\n");

    block6 = (char *) kmalloc(101000000);
    kprintf("kmalloc(101000000): %x\n", block6); // should return null
    print_free_list();
    kprintf("------------------------------------\n");


    kprintf("kfree(%x): %x\n", block1, kfree(block1)); // should return 1
    print_free_list();
    block1 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n\n", block1); // should be first block before hole
    kprintf("kfree(%x): %x\n", block2, kfree(block2)); // should return 1
    block2 = (char *) kmalloc(1010000);
    print_free_list();
    kprintf("kmalloc(1010000): %x\n", block2); // should be first block after hole
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block1, kfree(block4)); // should return 1
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block3, kfree(block2)); // should return 1
    print_free_list();
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block2, kfree(block1)); // should return 1
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block4, kfree(block3)); // should return 1
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block5, kfree(block5)); // should return 1
    kprintf("------------------------------------\n");
    print_free_list();


    for (i = 0; i < 3000000; i++);

    block1 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n", block1); // should be first block before hole
    kprintf("------------------------------------\n");
    block2 = (char *) kmalloc(1010000);
    kprintf("kmalloc(1010000): %x\n", block2); // should be first block after hole
    print_free_list();
    kprintf("------------------------------------\n");

    block3 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n", block3); // should be a block before hole
    print_free_list();
    kprintf("------------------------------------\n");

    block4 = (char *) kmalloc(1010000);
    kprintf("kmalloc(1010000): %x\n", block4); // should be last block after hole
    print_free_list();
    kprintf("------------------------------------\n");

    block5 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n", block5); // should be a block before hole
    print_free_list();
    kprintf("------------------------------------\n");

    block6 = (char *) kmalloc(101000000);
    kprintf("kmalloc(101000000): %x\n", block6); // should return null
    print_free_list();
    kprintf("------------------------------------\n");


    kprintf("kfree(%x): %x\n", block1, kfree(block1)); // should return 1
    print_free_list();
    block1 = (char *) kmalloc(1000);
    kprintf("kmalloc(1000): %x\n\n", block1); // should be first block before hole
    kprintf("kfree(%x): %x\n", block2, kfree(block2)); // should return 1
    block2 = (char *) kmalloc(1010000);
    print_free_list();
    kprintf("kmalloc(1010000): %x\n", block2); // should be first block after hole
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block1, kfree(block1)); // should return 1
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block3, kfree(block3)); // should return 1
    print_free_list();
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block2, kfree(block2)); // should return 1
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block4, kfree(block4)); // should return 1
    kprintf("------------------------------------\n");
    kprintf("kfree(%x): %x\n", block5, kfree(block5)); // should return 1
    kprintf("------------------------------------\n");
    print_free_list();

}
