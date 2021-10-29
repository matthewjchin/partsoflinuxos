# Project 3: Memory Allocator

To compile and use the allocator:

```bash
make
LD_PRELOAD=$(pwd)/allocator.so ls /
```

(in this example, the command `ls /` is run with the custom memory allocator instead of the default).

## Testing

To execute the test cases, use `make test`. To pull in updated test cases, run `make testupdate`. You can also run a specific test case instead of all of them:

```
# Run all test cases:
make test

# Run a specific test case:
make test run=4

# Run a few specific test cases (4, 8, and 12 in this case):
make test run='4 8 12'
```

===============================================================================

Name: Matthew Chin (GitHub Username: matthewjchin)

Project #3 Explanation:

The program is a memory allocator that will allocate memory, reuse, clear, free,
and reallocate memory with several different functions that will map memory and 
data based on what is being called. If reusing available memory space is needed,
it will be fitted accordingly to find the first, best, or worst empty region 
that can be filled, if the allocated size meets the needs of unused memory 
regions or unused parts of memory regions. 


================================================================================

Functions in allocator.c
--------------------------------------------------------------------------------

These functions are used to show memory and write to a file. 
print_memory:
    -Prints out the specific memory function
    -Shows address and specific linked list structure details such as region
    size, location address, successful allocation, and number of pages that 
    are used among other details. 

write_memory:
    -Writes same contents of print_memory to a file 


These functions are used to determine which algorithm is efficient enough for 
adding more memory regions via linked lists (or blocks) 
reuse:
    -Checks if the memory block has any free space
    -Uses first, best, or worst algorithm to put additional memory blocks of
    different sizes into that mapped region. 
    -Returns NULL if otherwise


These are the functions for dealing with memory allocation. 
malloc:
    -Allocate memory and gets a pointer to that memory location
    -Check to see if any memory blocks can be reused

free:
    -Frees any used memory so as to prevent any segmentation faults. 
    -Clears memory and offers opportunity to create visual picture of 
    the memory being allocated and replaced with other blocks. 
    
calloc:
   -While free appears to clear the memory it does not 
   actually "clear" details of memory
   -Need to set everything to 0 instead of leaving out wasted memory

realloc:
    -Changes the size of the memory block pointed to by ptr to size bytes. 
    -Frees the pointer and like malloc also re-aligns the memory blocks such that 
    each request is 8 bytes each. 
    -Allocates memory if there is a null pointer to this function with respect to 
    the proposed size. 



Visualization of the Memory Allocation:

Go to: https://github.com/usf-cs326-sp19/P3-sigsegv-factory/blob/master/output_vis.png



