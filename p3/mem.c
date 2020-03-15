////////////////////////////////////////////////////////////////////////////////
// Main File:        mem.c - mem allocation and freeing
// This File:        mem.c
// Other Files:      mem.h
// Semester:         CS 354 Spring 2019
//
// Author:           Bryce Van Camp
// Email:            bvancamp@wisc.edu
// CS Login:         bvan-camp
//
/////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
//                   fully acknowledge and credit all sources of help,
//                   other than Instructors and TAs.
//
// Persons:          none
//
// Online sources:   none
//////////////////////////// 80 columns wide ///////////////////////////////////

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include "mem.h"

#define A_BIT 1
#define P_BIT 2
#define PA_BITS 3
#define ALL_BUT_PA_BITS 0xFFFFFFFC
#define ALL_BUT_P_BIT 0xFFFFFFFD
#define ALL_BUT_A_BIT 0xFFFFFFFE
#define END_OF_HEAP ((BYTE*)start_block + heap_size)
#define HEAP_REMAINING (heap_size - heap_used)

#define GET_BLOCK_SIZE(block) (block->size_status & ALL_BUT_PA_BITS)
#define GET_PA_BITS(block) (block->size_status & PA_BITS)

#define FIND_PREV_BLOCK(block) ((block_header*)((BYTE*)block - ((block_header*)block - 1)->size_status))
#define FIND_NEXT_BLOCK(block) ((block_header*)((BYTE*)block + GET_BLOCK_SIZE(block)))

#define IS_ALLOCD(block) (block->size_status & A_BIT)
#define IS_PREV_ALLOCD(block) (block->size_status & P_BIT)

#define TRUE 1
#define FALSE 0

typedef char BOOL;
typedef unsigned char BYTE;

/*
 * This structure serves as the header for each allocated and free block.
 * It also serves as the footer for each free block but only containing size.
 */
typedef struct block_header
{
	int size_status;
	/*
	* Size of the block is always a multiple of 8.
	* Size is stored in all block headers and free block footers.
	*
	* Status is stored only in headers using the two least significant bits.
	*   Bit0 => least significant bit, last bit
	*   Bit0 == 0 => free block
	*   Bit0 == 1 => allocated block
	*
	*   Bit1 => second last bit
	*   Bit1 == 0 => previous block is free
	*   Bit1 == 1 => previous block is allocated
	*
	* End Mark:
	*  The end of the available memory is indicated using a size_status of 1.
	*
	* Examples:
	*
	* 1. Allocated block of size 24 bytes:
	*    Header:
	*      If the previous block is allocated, size_status should be 27
	*      If the previous block is free, size_status should be 25
	*
	* 2. Free block of size 24 bytes:
	*    Header:
	*      If the previous block is allocated, size_status should be 26
	*      If the previous block is free, size_status should be 24
	*    Footer:
	*      size_status should be 24
	*/
} block_header;

/* Global variable - DO NOT CHANGE. It should always point to the first block,
 * i.e., the block at the lowest address.
 */

block_header *start_block = NULL;
static int heap_size = 0;
static int heap_used = 0;

/*
 * This inline function sets the p-bit of the next block appropriately.
 *
 * cur_block this is the block header that is passed in
 */
static inline void Set_Next_PBit(block_header *cur_block)
{
	block_header *next_block = FIND_NEXT_BLOCK(cur_block);
	//ensure that next_block is inside the heap space
	if (next_block < END_OF_HEAP)
	{
		//set p-bit to 1 if current block is alloc'd
		if (IS_ALLOCD(cur_block)) next_block->size_status |= P_BIT;
		//else set p-bit to 0
		else next_block->size_status &= ALL_BUT_P_BIT;
	}
}

/*
 * Function for allocating 'size' bytes of heap memory.
 * Argument size: requested size for the payload
 * Returns address of allocated block on success.
 * Returns NULL on failure.
 * This function should:
 * - Check size - Return NULL if not positive or if larger than heap space.
 * - Determine block size rounding up to a multiple of 8 and possibly adding padding as a result.
 * - Use BEST-FIT PLACEMENT POLICY to find the block closest to the required block size
 * - Use SPLITTING to divide the chosen free block into two if it is too large.
 * - Update header(s) and footer as needed.
 * Tips: Be careful with pointer arithmetic.
 */
void* Alloc_Mem(int size)
{
	//init heap_size to store size of heap space
	if (heap_size == 0) heap_size = GET_BLOCK_SIZE(start_block);

	//Check size: Return NULL if not positive.
	if (size <= 0)
		return NULL;

	//Determine block size rounding up to a multiple of 8 and possibly adding padding as a result.
	int size_before_padding = sizeof(block_header) + size;
	int pad_size = (size_before_padding % 8 == 0) ? 0 : 8 - (size_before_padding % 8);
	int size_needed = size_before_padding + pad_size;

	//Check size_needed: Return NULL if larger than remaining heap space.
	if (size_needed > HEAP_REMAINING)
		return NULL;

	//Use BEST-FIT PLACEMENT POLICY to find the block closest to the required block size
	block_header *best_fit_block = NULL;
	for (block_header *cur_block = start_block; cur_block < END_OF_HEAP;
		cur_block = FIND_NEXT_BLOCK(cur_block))
	{
		//if current block is alloc'd, go to next block in loop
		if (IS_ALLOCD(cur_block)) continue;

		//if current block's size is exactly what was requested, 
			//set best_fit_block to cur_block and break
		if (GET_BLOCK_SIZE(cur_block) == size_needed)
		{
			best_fit_block = cur_block;
			break;
		}

		//store the best fit block that we've found so far; first part checks if larger by 
			//at least 8 bytes, second part checks if smaller than the current best-fit block
		if ((GET_BLOCK_SIZE(cur_block) >= size_needed + 8) && (best_fit_block == NULL ? TRUE :
			GET_BLOCK_SIZE(cur_block) < GET_BLOCK_SIZE(best_fit_block)))
		{
			best_fit_block = cur_block;
		}
	}

	//return NULL if no block was found with a large enough size for the request
	if (best_fit_block == NULL)
		return NULL;

	//alloc without splitting if block is exactly the requested size
	if (GET_BLOCK_SIZE(best_fit_block) == size_needed)
	{
		//set a-bit of best_fit_block to 1
		best_fit_block->size_status |= A_BIT;
		//update p-bit of the next block to specify that best_fit_block is alloc'd
		Set_Next_PBit(best_fit_block);
	}
	//else use SPLITTING to divide the chosen free block into two if it is too large.
	else if (GET_BLOCK_SIZE(best_fit_block) >= size_needed + 8)
	{
		//init new free block via splitting
		block_header *new_free_block = (BYTE*)best_fit_block + size_needed;

		//set header of new free block
		new_free_block->size_status = GET_BLOCK_SIZE(best_fit_block) - size_needed;
		new_free_block->size_status |= P_BIT;

		//set value of new free block's footer, i.e. size of block
		*((BYTE*)new_free_block + GET_BLOCK_SIZE(new_free_block) - sizeof(int)) =
			GET_BLOCK_SIZE(new_free_block);

		//update header of best_fit_block
		best_fit_block->size_status = GET_PA_BITS(best_fit_block) + size_needed;
		best_fit_block->size_status |= A_BIT;
	}
	//this else should never be entered but is a just-in-case measure
	else return NULL;

	//update amount of heap space used
	heap_used += size_needed;

	//return payload start addr of best fit block
	return best_fit_block + 1;
}

/*
 * Function for freeing up a previously allocated block.
 * Argument ptr: address of the block to be freed up.
 * Returns 0 on success.
 * Returns -1 on failure.
 * This function should:
 * - Return -1 if ptr is NULL.
 * - Return -1 if ptr is not a multiple of 8.
 * - Return -1 if ptr is outside of the heap space.
 * - Return -1 if ptr block is already freed.
 * - USE IMMEDIATE COALESCING if one or both of the adjacent neighbors are free.
 * - Update header(s) and footer as needed.
 */
int Free_Mem(void *ptr)
{
	//Return -1 if ptr is NULL, is not a multiple of 8, or is outside the heap space.
	if (ptr == NULL || (int)ptr % 8 != 0 || (ptr < start_block || ptr >= END_OF_HEAP))
		return -1;

	//init cur_block to be the header of ptr
	block_header *cur_block = (block_header*)ptr - 1;

	//Return -1 if the block is already freed.
	if (!IS_ALLOCD(cur_block))
		return -1;

	//if alloc'd or outside of heap space, don't coalesce; else assign blocks to coalesce
	block_header *prev_block = (IS_PREV_ALLOCD(cur_block) || FIND_PREV_BLOCK(cur_block)
		< start_block) ? NULL : FIND_PREV_BLOCK(cur_block);
	block_header *next_block = (IS_ALLOCD(FIND_NEXT_BLOCK(cur_block)) || FIND_NEXT_BLOCK(cur_block)
		>= END_OF_HEAP) ? NULL : FIND_NEXT_BLOCK(cur_block);

	//USE IMMEDIATE COALESCING if one or both of the adjacent neighbors are free.
	if (prev_block != NULL)
	{
		//coalesce prev_block and cur_block, update cur_block start addr
		prev_block->size_status += GET_BLOCK_SIZE(cur_block);
		cur_block = prev_block;
	}
	if (next_block != NULL)
	{
		//coalesce cur_block and next_block
		cur_block->size_status += GET_BLOCK_SIZE(next_block);
	}

	//Update header(s) and footer as needed.

	//set cur_block's a-bit to 0
	cur_block->size_status &= ALL_BUT_A_BIT;

	//assign cur_block's footer and its value
	int *cur_footer = (BYTE*)cur_block + GET_BLOCK_SIZE(cur_block) - sizeof(int);
	*cur_footer = GET_BLOCK_SIZE(cur_block);

	//zero out p-bit of next block if it's within the heap space
	Set_Next_PBit(cur_block);

	//update amount of heap space used
	heap_used -= cur_block->size_status;

	//Returns 0 on success.
	return 0;
}

/*
 * Function used to initialize the memory allocator.
 * Intended to be called ONLY once by a program.
 * Argument sizeOfRegion: the size of the heap space to be allocated.
 * Returns 0 on success.
 * Returns -1 on failure.
 */
int Init_Mem(int sizeOfRegion) {
	int pagesize;
	int padsize;
	int fd;
	int alloc_size;
	void* space_ptr;
	block_header* end_mark;
	static int allocated_once = 0;

	if (0 != allocated_once) {
		fprintf(stderr,
			"Error:mem.c: Init_Mem has allocated space during a previous call\n");
		return -1;
	}
	if (sizeOfRegion <= 0) {
		fprintf(stderr, "Error:mem.c: Requested block size is not positive\n");
		return -1;
	}

	// Get the pagesize
	pagesize = getpagesize();

	// Calculate padsize as the padding required to round up sizeOfRegion 
	// to a multiple of pagesize
	padsize = sizeOfRegion % pagesize;
	padsize = (pagesize - padsize) % pagesize;

	alloc_size = sizeOfRegion + padsize;

	// Using mmap to allocate memory
	fd = open("/dev/zero", O_RDWR);
	if (-1 == fd) {
		fprintf(stderr, "Error:mem.c: Cannot open /dev/zero\n");
		return -1;
	}
	space_ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE,
		fd, 0);
	if (MAP_FAILED == space_ptr) {
		fprintf(stderr, "Error:mem.c: mmap cannot allocate space\n");
		allocated_once = 0;
		return -1;
	}

	allocated_once = 1;

	// for double word alignment and end mark
	alloc_size -= 8;

	// To begin with there is only one big free block
	// initialize heap so that start block meets 
	// double word alignement requirement
	start_block = (block_header*)space_ptr + 1;
	end_mark = (block_header*)((void*)start_block + alloc_size);

	// Setting up the header
	start_block->size_status = alloc_size;

	// Marking the previous block as used
	start_block->size_status += 2;

	// Setting up the end mark and marking it as used
	end_mark->size_status = 1;

	// Setting up the footer
	block_header *footer = (block_header*)((char*)start_block + alloc_size - 4);
	footer->size_status = alloc_size;

	return 0;
}

/*
 * Function to be used for DEBUGGING to help you visualize your heap structure.
 * Prints out a list of all the blocks including this information:
 * No.      : serial number of the block
 * Status   : free/used (allocated)
 * Prev     : status of previous block free/used (allocated)
 * t_Begin  : address of the first byte in the block (where the header starts)
 * t_End    : address of the last byte in the block
 * t_Size   : size of the block as stored in the block header
 */
void Dump_Mem() {
	int counter;
	char status[5];
	char p_status[5];
	char *t_begin = NULL;
	char *t_end = NULL;
	int t_size;

	block_header *current = start_block;
	counter = 1;

	int used_size = 0;
	int free_size = 0;
	int is_used = -1;

	fprintf(stdout, "************************************Block list***\
                    ********************************\n");
	fprintf(stdout, "No.\tStatus\tPrev\tt_Begin\t\tt_End\t\tt_Size\n");
	fprintf(stdout, "-------------------------------------------------\
                    --------------------------------\n");

	while (current->size_status != 1) {
		t_begin = (char*)current;
		t_size = current->size_status;

		if (t_size & 1) {
			// LSB = 1 => used block
			strcpy(status, "used");
			is_used = 1;
			t_size = t_size - 1;
		}
		else {
			strcpy(status, "Free");
			is_used = 0;
		}

		if (t_size & 2) {
			strcpy(p_status, "used");
			t_size = t_size - 2;
		}
		else {
			strcpy(p_status, "Free");
		}

		if (is_used)
			used_size += t_size;
		else
			free_size += t_size;

		t_end = t_begin + t_size - 1;

		fprintf(stdout, "%d\t%s\t%s\t0x%08lx\t0x%08lx\t%d\n", counter, status,
			p_status, (unsigned long int)t_begin, (unsigned long int)t_end, t_size);

		current = (block_header*)((char*)current + t_size);
		counter = counter + 1;
	}

	fprintf(stdout, "---------------------------------------------------\
                    ------------------------------\n");
	fprintf(stdout, "***************************************************\
                    ******************************\n");
	fprintf(stdout, "Total used size = %d\n", used_size);
	fprintf(stdout, "Total free size = %d\n", free_size);
	fprintf(stdout, "Total size = %d\n", used_size + free_size);
	fprintf(stdout, "***************************************************\
                    ******************************\n");
	fflush(stdout);

	return;
}
