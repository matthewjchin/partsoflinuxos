/**
 * allocator.c
 * ==============================================================================
 * Explores memory management at the C runtime level.
 *
 * Author: Matthew Chin (matthewjchin)
 *
 * To use (one specific command):
 * LD_PRELOAD=$(pwd)/allocator.so command
 * ('command' will run with your allocator)
 *
 * To use (all following commands):
 * export LD_PRELOAD=$(pwd)/allocator.so
 * (Everything after this point will use your custom allocator -- be careful!)
 * ==============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <limits.h>

#ifndef DEBUG
#define DEBUG 1
#endif


#define LOG(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

#define LOGP(str) \
    do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): %s", __FILE__, \
            __LINE__, __func__, str); } while (0)

/**
 * struct mem_block
 * ==============================================================================
 * This struct represents one block of memory to be placed inside a region. 
 * 
 * ==============================================================================
 */
struct mem_block {
    /* Each allocation is given a unique ID number. If an allocation is split in
     * two, then the resulting new block will be given a new ID. */
    unsigned long alloc_id;

    /* Size of the memory region */
    size_t size;

    /* Space used; if usage == 0, then the block has been freed. */
    size_t usage;

    /* Pointer to the start of the mapped memory region. This simplifies the
     * process of finding where memory mappings begin. */
    struct mem_block *region_start;

    /* If this block is the beginning of a mapped memory region, the region_size
     * member indicates the size of the mapping. In subsequent (split) blocks,
     * this is undefined. */
    size_t region_size;

    /* Next block in the chain */
    struct mem_block *next;
};

/* Start (head) of our linked list: */
struct mem_block *g_head = NULL;

/* Allocation counter: */
unsigned long g_allocations = 0;

pthread_mutex_t g_alloc_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * print_memory
 * ===========================================================================
 * 
 * TODO: Avoid allocating memory for printf
 *
 * Prints out the current memory state, including both the regions and blocks.
 * Entries are printed in order, so there is an implied link from the topmost
 * entry to the next, and so on.
 * 
 * ===========================================================================
 */
void print_memory(void)
{
    puts("-- Current Memory State --");
    struct mem_block *current_block = g_head;
    struct mem_block *current_region = NULL;
    while (current_block != NULL) {
        if (current_block->region_start != current_region) {
            current_region = current_block->region_start;
            printf("[REGION] %p-%p %zu\n",
                    current_region,
                    (void *) current_region + current_region->region_size,
                    current_region->region_size);
        }
        printf("[BLOCK]  %p-%p (%ld) %zu %zu %zu\n",
                current_block,
                (void *) current_block + current_block->size,
                current_block->alloc_id,
                current_block->size,
                current_block->usage,
                current_block->usage == 0
                    ? 0 : current_block->usage - sizeof(struct mem_block));
        current_block = current_block->next;
    }
    // free(current_block);
}

/**
 * write_memory
 * ====================================================
 * -File Logging
 * -Writes output from print_memory to a file
 * ====================================================
 */
void write_memory(FILE *all_output){
    puts("-- Current Memory State --");
    struct mem_block *current_block = g_head;
    struct mem_block *current_region = NULL;
    while (current_block != NULL) {
        if (current_block->region_start != current_region) {
            current_region = current_block->region_start;
            fprintf(all_output,"[REGION] %p-%p %zu\n",
                    current_region,
                    (void *) current_region + current_region->region_size,
                    current_region->region_size);
        }
        fprintf(all_output,"[BLOCK]  %p-%p (%ld) %zu %zu %zu\n",
                current_block,
                (void *) current_block + current_block->size,
                current_block->alloc_id,
                current_block->size,
                current_block->usage,
                current_block->usage == 0
                    ? 0 : current_block->usage - sizeof(struct mem_block));
        current_block = current_block->next;
    }
}

/**
 * reuse
 * ======================================================================
 * Checks to see if any free space available in the block of memory 
 * 
 * ======================================================================
 */
void *reuse(size_t size) {
    // TODO: using free space management algorithms, find a block of memory that
    // we can reuse. Return NULL if no suitable block is found.      
    char *algo = getenv("ALLOCATOR_ALGORITHM");
    if (algo == NULL) {
         algo = "first_fit";
        // algo = "best_fit";
        // algo = "worst_fit";
    }       
    struct mem_block *stuff = g_head;

    if (strcmp(algo, "first_fit") == 0) {
        LOG("ADDRESS OF CURR: %p\n", stuff);
        while(stuff != NULL){
            LOG("Size and Usage of Stuff: %zu %zu\n", stuff->size, stuff->usage);
            if((stuff->size - stuff->usage) > (size + sizeof(struct mem_block))){
               return stuff; 
            }
            stuff = stuff->next;
        }
        return stuff;

    } else if (strcmp(algo, "best_fit") == 0) {
        size_t usage = INT16_MAX;
        struct mem_block *new_stuff = NULL;
        while(stuff != NULL){
            if((stuff->size - stuff->usage) >= (size + sizeof(struct mem_block))){
                if(stuff->size - stuff->usage <= usage) {
                    usage = stuff->size - stuff->usage;
                    new_stuff = stuff;
                }
            }
            stuff = stuff->next;
        }
        return new_stuff;

    } else if (strcmp(algo, "worst_fit") == 0) {
        size_t small_usage = 0;
        struct mem_block *new_stuff = NULL;
        while(stuff != NULL){
            if((stuff->size - stuff->usage) >= (size + sizeof(struct mem_block))){    
                if((stuff->size - stuff->usage) >= small_usage) {
                    small_usage = stuff->size - stuff->usage;
                    new_stuff = stuff;
                }
            }
            stuff = stuff->next;
        }
        return new_stuff;

    }
    

    // return NULL if there are no memory regions that can be filled up
    return NULL;
}

/**
 * malloc
 * =======================================================================
 * -Allocate memory
 * -Has a pointer that directs to a location of the allocated memory
 * 
 * -Check first to see if any memory blocks in use can be reused
 * -Map new memory region if no memory blocks are used
 * 
 * =======================================================================
 */
void *malloc(size_t propd_size)
{
    // TODO: allocate memory. You'll first check if you can reuse an existing
    // block. If not, map a new memory region.
    
    /* Go through list and see if there are any free blocks that actually fit 
     * what is going to be allocated into memory. */
    
    pthread_mutex_lock(&g_alloc_mutex);

    LOG("Allocation request; size = %zu\n", propd_size);
    if (propd_size % 8 != 0){
        propd_size = propd_size + (8 - propd_size % 8);
    }

    struct mem_block *check_reusable = NULL;
    check_reusable = reuse(propd_size);
    
    if (check_reusable != NULL){ 

        size_t real_sz = propd_size + sizeof(struct mem_block);
        if (check_reusable->usage == 0){
            check_reusable->alloc_id = g_allocations++;
            check_reusable->size = real_sz;
            if (getenv("ALLOCATOR_SCRIBBLE") != NULL && (strcmp(getenv("ALLOCATOR_SCRIBBLE"), "1") == 0)){
                memset(check_reusable+1, 0xAA, propd_size);
            }

            pthread_mutex_unlock(&g_alloc_mutex);
            return check_reusable + 1;
        } else {
            // Create a new block this_block that makes a new mapped memory region
            struct mem_block *this_block = (void *) check_reusable + check_reusable->usage;
            this_block->alloc_id = g_allocations++;
            this_block->size = check_reusable->size - check_reusable->usage;
            this_block->usage = real_sz;
            this_block->region_start = check_reusable->region_start;
            this_block->region_size = check_reusable->region_size;
            this_block->next = check_reusable->next;

            
            // Update the reusable block's size to become its usage
            check_reusable->size = check_reusable->usage;
            check_reusable->next = this_block;

            // Check for Scribbling
            if((getenv("ALLOCATOR_SCRIBBLE") != NULL) && (strcmp(getenv("ALLOCATOR_SCRIBBLE"), "1") == 0)){
                memset(this_block + 1, 0xAA, propd_size);
            }

            pthread_mutex_unlock(&g_alloc_mutex);
            return this_block + 1;
        }
        
    }
    
    // Real size is Sum of proposed size and size of a mem_block struct
    size_t real_sz = propd_size + sizeof(struct mem_block);
    int page_sz = getpagesize();
    size_t num_of_pages = real_sz / page_sz;
    if((real_sz % page_sz) != 0){
        num_of_pages++;
    }
    // New region size to be set to product of number of pages and page size
    size_t region_sz = num_of_pages * page_sz;
    
    // mmap in alloc requests for a new memory region from the kernel
    struct mem_block *block = mmap(
        NULL, /* Address (we use NULL to let the kernel decide) */
        region_sz, /* Size of memory block to allocate */
        PROT_READ | PROT_WRITE, /* Memory protection flags */
        MAP_PRIVATE | MAP_ANONYMOUS, /* Type of mapping */
        -1, /* file descriptor */
        0 /* Offset to start at within the file */);

    if (block == MAP_FAILED){
        perror("mmap");
        pthread_mutex_unlock(&g_alloc_mutex);
        return NULL;
    }

    block->alloc_id = g_allocations++;
    block->size = region_sz;
    block->region_start = block;
    block->region_size = region_sz;
    block->usage = real_sz;  // Usage
    block->next = NULL;   

    if (g_head == NULL){
        g_head = block;
    } else{
        struct mem_block *curr = g_head;
        while (curr != NULL){
            if (curr->next == NULL){
                // LOG("Linking %p to %p\n", curr, block);
                curr->next = block;
                break;
            }
	        curr = curr->next;
        }
    }

    // Check Scribbling for newly mapped region and overwrite the block/region
    // if(getenv("ALLOCATOR_SCRIBBLE")){
    if((getenv("ALLOCATOR_SCRIBBLE") != NULL) && (strcmp(getenv("ALLOCATOR_SCRIBBLE"), "1") == 0)){

        memset(block + 1, 0xAA, propd_size);
    }

    // Unlock the thread
    pthread_mutex_unlock(&g_alloc_mutex);
    return block + 1;
}

/**
 * free
 * ====================================================================
 * Frees any used memory so as to prevent any segmentation faults. 
 * Clears memory and offers opportunity to create visual picture of 
 * the memory being allocated and replaced with other blocks. 
 * ==================================================================== 
 */
void free(void *ptr)
{   
    /* 
        This is separate code that is used to send all information from the 
        memory output to a text file. This text file will call the script
        file in tests/viz in order to create a .png image file showing the
        visualization of the memory allocation program. 
    */

    if (ptr == NULL) {
        /* Freeing a NULL pointer does nothing */
        return;
    }

    // FILE *file_pic = fopen("tests/viz/mem.txt", "w");
    // write_memory(file_pic);

    pthread_mutex_lock(&g_alloc_mutex);

    // Check to see if the block can be freed if applicable
    bool check_to_free = true; 
   

    struct mem_block *block = (struct mem_block*) ptr - 1;
    // LOG("Free request on allocation = %lu\n", block->alloc_id);
    block->usage = 0;

    // TODO: algorithm for figuring out if we can free a region:
    // 1. go to region start                                    
    // 2. traverse through the linked list                      
    // 3. stop when you:                                       
    //     a. find something that's not free                    
    //     b. when you find the start of a different region     
    // 4. if (a) move on; if (b) then munmap 
    struct mem_block *curr_block = block->region_start;
    // LOG("Other Block = %p\n", block->region_start);

    while (curr_block != NULL && (curr_block->region_start == block->region_start)) {
        if (curr_block->usage != 0){
            check_to_free = false;
            // pthread_mutex_unlock(&g_alloc_mutex);
            // return;
	        break;
        }
        curr_block = curr_block->next;
    }
    /* Update the linked list */      
    if (check_to_free){
        if (g_head == block->region_start) {                                        
            g_head = curr_block;                                     
        } else {                                                    
            struct mem_block *temp = g_head;                        
            while (temp->next != NULL && (temp->next != block->region_start)){
                temp = temp->next;                                  
            }     
            LOG("Temporary block: %p\n", temp);	
            temp->next = curr_block;                                 
        }                                                           
                                                                
        // TODO: free memory. If the containing region is empty (i.e., there are no
        // more blocks in use), then it should be unmapped.

        int ret = munmap(block->region_start, block->region_size);
        if (ret == -1) {                                            
            perror("munmap");                                       
        }  
    }                                                      
   
    pthread_mutex_unlock(&g_alloc_mutex);
}


/**
 * calloc
 * ==================================================================== 
 * While free appears to clear the memory it does not 
 * actually "clear" details of memory
 * 
 * Need to set everything to 0 instead of leaving out wasted memory
 * 
 * ====================================================================
 */
void *calloc(size_t nmemb, size_t size)
{
    // size_t check_size = nmemb * size;
    void *ptr = malloc(nmemb * size);
    // memset(ptr, 0, check_size);
    memset(ptr, 0, nmemb * size);
    return ptr;
}


/**
 * realloc
 * ================================================================================
 * Changes the size of the memory block pointed to by ptr to size bytes. 
 * 
 * Frees the pointer and like malloc also re-aligns the memory blocks such that 
 * each request is 8 bytes each. 
 * 
 * Allocates memory if there is a null pointer to this function with respect to 
 * the proposed size. 
 * ================================================================================
 */
void *realloc(void *ptr, size_t size)
{
    if (size % 8 != 0) {                              
        size = size + (8 - size % 8);                 
        LOG("Aligned size: %zu\n", size);             
    }                                                 
    if (ptr == NULL) {
        return malloc(size);
    }
    // TODO: reallocation logic
    if (size == 0){
        free(ptr);
        return NULL;
    }

    return NULL;
}




