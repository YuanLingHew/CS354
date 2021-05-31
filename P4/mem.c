/******************************************************************************
 * FILENAME: mem.c
 * AUTHOR:   cherin@cs.wisc.edu <Cherin Joseph>
 * DATE:     20 Nov 2013
 * EDITED: for CS354 UW Madison Spring 2021 - Michael Doescher
 * PROVIDES: Contains a set of library functions for memory allocation
 * *****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "mem.h"

// fitting policy
enum POLICY policy;

/* The BLOCK_HEADER structure serves as the header for each block.
 * 
 * The heaaders we are using for this project are similar to those described 
 * in the book for the implicit free list in section 9.9.6, and in lecture
 * In the implicit free packs both the size and allocated bit in one int.
 * The size in the implicit free list includes the size of the header.
 *
 * In this project we're going to use a struct that tracks additional 
 * information in the block header.
 * 
 * The first piece of information is a 'packed_pointer' that combines the 
 * absolute location (a memory address) of the next header and the alloc bit
 * The headers must begin on an address divisible by 4. This means the last 
 * two bits must be 0.  We use the least significant bit (LSB) to indicate 
 * if the block is free: LSB = 0; or allocated LSB = 1.
 *
 * The value stored in the size variable is either the size requested by 
 * the user for allocated blocks, or the available payload size (not including 
 * the size of the header) for free blocks.
 * This will allow us to calculate the memory utilization.  Memory utilization is
 * the requested_size / (padding + header_size).  
 * The provided function Mem_Dump takes care of this calculation for us.
 *
 * The end of the list (the last header) has the packed_pointer  set to NULL,
 * and the size set to 0.
 */

typedef struct BLOCK_HEADER {
  void *packed_pointer; // address of the next header + alloc bit.
  unsigned size;
} BLOCK_HEADER;

// initialize helper functions
BLOCK_HEADER *first_header; // this global variable is a pointer to the first header
int Is_Allocated(BLOCK_HEADER *);
int Is_Free(BLOCK_HEADER *);
BLOCK_HEADER * Get_Next_Header_After_Alloc(BLOCK_HEADER *, unsigned);
int Split_Check (BLOCK_HEADER *, unsigned);
BLOCK_HEADER * Get_Next_Header(BLOCK_HEADER *);
int Get_Size(BLOCK_HEADER *);
void * Get_User_Pointer(unsigned, BLOCK_HEADER *);
void Set_Allocated(BLOCK_HEADER *);
void Set_Free(BLOCK_HEADER *);
void Set_Size(BLOCK_HEADER *, BLOCK_HEADER *);
unsigned Get_Padding_Size(BLOCK_HEADER *, void *, unsigned);
void Set_Next_Header(BLOCK_HEADER *, BLOCK_HEADER *);
BLOCK_HEADER * Get_Header_From_User_Pointer(void *);

// *********************************************************************************
// *********************************************************************************
// *********************************************************************************

/*  We recommend you write some helper functions to unpack the headers 
 *  and retrieve specific pieces of data. I wrote functions named:
 *  
 *  1) Is_Allocated // return 1 if allocated 0 if not
 *  2) Is_Free      // return 1 if free 0 if not
 *  3) Get_Next_Header // unpacks the header and returns a pointer to the  
 *  the next header, NULL is this is the last BLOCK_HEADER
 *  4) Get_Size 
 *  5) Get_User_Pointer // the pointer that the user can write data to
 *  6) Get_Header_From_User_Pointer // the pointer that the user writes data to - used in Mem_Free
 *  7) Set_Next_Pointer
 *  8) Set_Allocated // set the allocated bit to 1
 *  9) Set_Free // set the allocated bit to 0
 *  10) Set_Size 
 */

// checks if block is allocated, returns NULL if its the last header block
int Is_Allocated(BLOCK_HEADER *header) {
  if (header -> packed_pointer == NULL) {
    return NULL;
  }
  
  if ((unsigned)header -> packed_pointer & 1) {
    return 1;
  }
  return 0;
}

// checks if block is free, returns NULL if its the last header block
int Is_Free(BLOCK_HEADER *header) {
  if (header -> packed_pointer == NULL) {
    return NULL;
  }
  
  if ((unsigned)header -> packed_pointer & 1){
    return 0;
  }
  return 1;
}

// gets pointer that points to the next header after memory allocation (used for splitting)
// if splitting is not done, returns pointer to next header
BLOCK_HEADER * Get_Next_Header_After_Alloc (BLOCK_HEADER *prev, unsigned total) {
  void * curr = (void *)prev + sizeof(BLOCK_HEADER) + total;
  unsigned avail = (unsigned)((void *)Get_Next_Header(prev) - (unsigned)curr);
  if (avail >= 12) {
    return (BLOCK_HEADER *)curr;
  }
  return (BLOCK_HEADER *)((unsigned)prev -> packed_pointer & 0xfffffffe);
}

// split check
int Split_Check (BLOCK_HEADER *prev, unsigned total) {
  void * curr = (void *)prev + sizeof(BLOCK_HEADER) + total;
  unsigned avail = (unsigned)((void *)Get_Next_Header(prev) - (unsigned)curr);
  if (avail >= 12) {
    return 1;
  }
  return 0;
}

// just gets the next header
// NULL if no next header (last header)
BLOCK_HEADER * Get_Next_Header (BLOCK_HEADER *curr) {
  if (curr -> size != 0) {
    BLOCK_HEADER * next = (BLOCK_HEADER*)((unsigned)curr -> packed_pointer & 0xfffffffe);
    return next;
  }
  return NULL;
}
  

// gets size of header
int Get_Size (BLOCK_HEADER *header) {
  return header -> size;
}

// get user pointer that points to first available address
void * Get_User_Pointer (unsigned sizeReq, BLOCK_HEADER *header) {
  BLOCK_HEADER *curr = header;
  while (curr -> packed_pointer != NULL) {
    BLOCK_HEADER *next = Get_Next_Header(curr);
    // if current block is big enough and free
    if (Is_Free (curr)  && ((curr -> size) >= sizeReq)) {
      return (void *)curr + sizeof(BLOCK_HEADER);
    }
    else {
      curr = next;
    }
  }
  return NULL;
}

// set alloc_bit of header block to 1
void Set_Allocated (BLOCK_HEADER *header) {
  header -> packed_pointer =(void *)((unsigned)header -> packed_pointer | 0x00000001); 
}

// set alloc_bit of header block to 0
void Set_Free (BLOCK_HEADER *header) {
  header -> packed_pointer = (void *)((((unsigned)header -> packed_pointer) & ~1) | 0); 
}

// sets size of block 
void Set_Size (BLOCK_HEADER *start, BLOCK_HEADER *end) {
  unsigned ret = (unsigned)end - (unsigned)start - sizeof(BLOCK_HEADER);
  start -> size = ret;
}

// gets the requires amount of padding
unsigned Get_Padding_Size (BLOCK_HEADER *end, void *start, unsigned payload) {
  unsigned curr = (unsigned)end;
  unsigned padding = 0;
  while (curr % 4 != 0) {
    curr += 1;
    padding += 1;
  }
  return padding;
}

// sets the next header of the current header
void Set_Next_Header (BLOCK_HEADER *start, BLOCK_HEADER *end) {
  start -> packed_pointer = (void *)end;
}

// gets the header from user pointer
BLOCK_HEADER * Get_Header_From_User_Pointer (void *userPointer) {
  return (void *)userPointer - sizeof(BLOCK_HEADER);
}



// *********************************************************************************
// *********************************************************************************
// *********************************************************************************

/* Function used to Initialize the memory allocator */
/* Do not change this function */
/* Written by Cherin Joseph modified by Michael Doescher */
/* Not intended to be called more than once by a program */
/* Argument - sizeOfRegion: Specifies the size of the chunk which needs to be allocated 
   	      policy: indicates the policy to use eg: best fit is 0*/
/* Returns 0 on success and -1 on failure */
/* Notes we're using mmap here instead of sbrk as in the book to take advantage of caching
 * as described in the OS lectures
 *
 * Study the end of the function where the headers are initialized for hints!
 */
int Mem_Init(int sizeOfRegion, enum POLICY policy_input)
{   
    policy = policy_input;
    
    int pagesize;
    int padsize;
    int fd;
    int alloc_size;
    void* space_ptr;
    static int allocated_once = 0;
    
    if(0 != allocated_once) {
      fprintf(stderr,"Error:mem.c: Mem_Init has allocated space during a previous call\n");
      return -1;}
    if(sizeOfRegion <= 0) {
        fprintf(stderr,"Error:mem.c: Requested block size is not positive\n");
        return -1;}
  
    /* Calculate padsize as the padding required to round up sizeOfRegion to a multiple of pagesize */
    pagesize = getpagesize(); //  Get the pagesize
    padsize = sizeOfRegion % pagesize;
    padsize = (pagesize - padsize) % pagesize;
    alloc_size = sizeOfRegion + padsize;
  
    /* Using mmap to allocate memory */
    fd = open("/dev/zero", O_RDWR);
    if(-1 == fd) {
      fprintf(stderr,"Error:mem.c: Cannot open /dev/zero\n");
      return -1;}
    space_ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (MAP_FAILED == space_ptr) {
      fprintf(stderr,"Error:mem.c: mmap cannot allocate space\n");
      allocated_once = 0;
      return -1;}
    
    allocated_once = 1;
    
    // To begin with, there is only one big, free block.
    // Initialize the first header */
    first_header = (BLOCK_HEADER*)space_ptr;
    // free size
    // Remember that the 'size' stored for free blocks excludes the space for the headers
    first_header->size = (unsigned)alloc_size - 2*sizeof(BLOCK_HEADER);
    // address of last header
    first_header->packed_pointer = (void *)first_header + alloc_size - sizeof(BLOCK_HEADER);

    // initialize last header
    // packed_pointers are void pointer, the headers are not
    BLOCK_HEADER *last_header = (BLOCK_HEADER *)first_header->packed_pointer;
    last_header->size = 0;
    last_header->packed_pointer = NULL;
    return 0;
}


// *********************************************************************************
// *********************************************************************************
// *********************************************************************************

/* Function for allocating 'size' bytes. */
/* Return the user writeable address of allocated block 
 * - this is the first byte of the payload, not the address of the header */
/* Return NULL on failure */

/* Here is what this function should accomplish */
/* - Check for sanity of size - Return NULL when appropriate - at least 1 byte*/
/* - Traverse the list of blocks and locate a free block which can accommodate 
 * the requested size based on the policy (e.g. first fit, best fit) */
/* - The next header must be aligned with an address divisible by 4. 
 *     - Add padding to accomodate this requirement. */
/* - When allocating a block - split it into two blocks when possible 
 *     - the allocated block should go first and the free block second
 *     - the free block must have a minimum payload size of 4 bytes 
 *     - do not split if the mininmum payload size can not be reserved */

/* Tips: Be careful with pointer arithmetic */
void* Mem_Alloc(int size) {

  /* Your code should go in here */
  // checks sanity of size
  if (size < 1) {
    return NULL;
  }
  
  // locates free block
  void *userPointer = Get_User_Pointer ((unsigned)size, first_header);
  // if no memory available
  if (userPointer == NULL) {
    return NULL;
  }
  
  // allocate memory
  // gets location to where user can allocate memory
  BLOCK_HEADER *userPointerHead = Get_Header_From_User_Pointer(userPointer);
  // calculates total size (size+padding)
  unsigned totalSize = (unsigned)size + Get_Padding_Size((void *)userPointer + (unsigned)size, userPointerHead, (unsigned)size);
  // determines next header
  BLOCK_HEADER *nextHead = Get_Next_Header_After_Alloc(userPointerHead, totalSize);
  // check if splitting was done
  // if split
  if (Split_Check(userPointerHead, totalSize)) {
    // sets size of next header
    Set_Size (nextHead, Get_Next_Header(userPointerHead));
    // sets next header's packed_pointer
    Set_Next_Header(nextHead, Get_Next_Header(userPointerHead));
    // sets next header's alloc_bit
    Set_Free(nextHead);
  }
  
  // updates occupied block header's packed_pointer (splitting)
  Set_Next_Header(userPointerHead, nextHead);
  // sets occupied block header's alloc_bit
  Set_Allocated(userPointerHead);
  // updates occupied block header's size
  userPointerHead -> size = size;

  return userPointer;
}

// *********************************************************************************
// *********************************************************************************
// *********************************************************************************

/* Function for freeing up a previously allocated block */
/* Argument - ptr: Address of the block to be freed up i
 *     - this is the first address of the payload */
/* Returns 0 on success */
/* Returns -1 on failure */
/* Here is what this function should accomplish */
/* - Return -1 if ptr is NULL */
/*  Return -1 if ptr is not pointing to the first byte of an allocated block
 *     - hint: check all block headers, determine if the alloc bit is set */
/* - Mark the block as free */
/* - Coalesce if one or both of the immediate neighbours are free */
int Mem_Free(void *ptr)
{
  /* Your code should go in here */
  // if ptr is null
  if (ptr == NULL) {
    return -1;
  }

  // find last header
  BLOCK_HEADER * lastHeader = first_header;
  while (lastHeader -> packed_pointer != NULL) {
    lastHeader = Get_Next_Header (lastHeader);
  }

  // if ptr is not in allocated heap range
  if ((unsigned)ptr < (unsigned)first_header || (unsigned)ptr > (unsigned)lastHeader) {
    return -1;
  }
  
  // if ptr is pointing to a free block
  BLOCK_HEADER * ptrHeader = (void *)ptr - sizeof(BLOCK_HEADER); 
  if (Is_Free(ptrHeader)) {
    return -1;
  }

  // checks if next header is empty
  if (Is_Free(Get_Next_Header(ptrHeader))) {
    BLOCK_HEADER * finalHeader = Get_Next_Header(ptrHeader);
    while (Is_Free(finalHeader)) {
      finalHeader = Get_Next_Header(finalHeader);
    }
    // immediate coalesce
    // update size
    Set_Size(ptrHeader, finalHeader);
    ptrHeader -> size = ((unsigned)ptrHeader -> size) - sizeof(BLOCK_HEADER);
    // update pointer to next header
    ptrHeader -> packed_pointer = (void *)finalHeader;
  }

  // set alloc_bit to 0
  Set_Free (ptrHeader);

  // deffered coalesce
  BLOCK_HEADER * currHeader = first_header;
  BLOCK_HEADER * tempHeader1 = NULL;
  int flag = 0;
  while (currHeader -> packed_pointer != NULL) {
    // if current block is free
    if (Is_Free(currHeader)) {
      // if flag = 0
      if (flag == 0) {
	tempHeader1 = currHeader;
	flag = 1;
      }
      // if flag = 1
      else if (flag == 1) {
	flag = 2;
      }
    }
    // if current block is allocated
    else {
      if (flag == 2) {
	// coalesce
	Set_Size(tempHeader1, currHeader);
	tempHeader1 -> packed_pointer = (void *)currHeader;
	Set_Free (tempHeader1);
	flag = 0;
      }

      else if (flag == 1) {
	flag = 0;
      }
    }
    
    currHeader = Get_Next_Header (currHeader);
  }

  // if currHeader reaches the last header where coalescence needs to be done
  if (flag == 2) {
    Set_Size(tempHeader1, currHeader);
    tempHeader1 -> packed_pointer = (void *)currHeader;
    Set_Free (tempHeader1);
  }
  
  
  return 0;
}

// *********************************************************************************
// *********************************************************************************
// *********************************************************************************

/* Function to be used for debugging */
/* Prints out a list of all the blocks along with the following information for each block */
/* No.      : Serial number of the block */
/* Status   : free/busy */
/* Begin    : Address of the first user allocated byte - i.e. start of the payload */ 
/* End      : Address of the last byte in the block (payload or padding) */
/* Payload  : Payload size of the block - the size requested by the user or free size */
/* Padding  : Padding size of the block */
/* T_Size   : Total size of the block (including the header, payload, and padding) */
/* H_Begin  : Address of the block header */
void Mem_Dump()
{
    unsigned id = 0;
    unsigned total_free_size = 0;
    unsigned total_payload_size = 0;
    unsigned total_padding_size = 0;
    unsigned total_used_size = sizeof(BLOCK_HEADER); // end of heap header not counted in loop below
    char status[5];
    unsigned payload = 0;
    unsigned padding = 0;
    BLOCK_HEADER *current = first_header;
    
    fprintf(stdout,"************************************Block list***********************************\n");
    fprintf(stdout,"%5s %7s %12s %12s %9s %9s %8s %12s\n", 
            "Id.", "Status", "Begin", "End", "Payload", "Padding", "T_Size", "H_Begin");
    fprintf(stdout,"---------------------------------------------------------------------------------\n");
            
    while (current->packed_pointer != NULL) {
        id++;
        BLOCK_HEADER *next = (BLOCK_HEADER *)((unsigned)current->packed_pointer & 0xfffffffe);
        void *begin = (void *)current + sizeof(BLOCK_HEADER);
        void *end = (void *)next - 1;
        
        if ((unsigned)current->packed_pointer & 1) { // allocated block
            strcpy(status, "Busy");
            payload = current->size;
            padding = (unsigned)((unsigned)next-(unsigned)current)-payload-sizeof(BLOCK_HEADER);
            total_payload_size += payload;
            total_padding_size += padding;
            total_used_size += payload + padding + sizeof(BLOCK_HEADER);
        }
        else { // free block 
            strcpy(status, "Free");
            payload = current->size;
            padding = 0;
            total_used_size += sizeof(BLOCK_HEADER);
            total_free_size += payload;
        }
        unsigned total_block_size = sizeof(BLOCK_HEADER) + padding + payload;
        
        fprintf(stdout,"%5d %7s %12p %12p %9u %9u %8u %12p\n", 
            id, status, begin, end, payload, padding, total_block_size, current);
        current = next;
    }
    fprintf(stdout,"---------------------------------------------------------------------------------\n");
    fprintf(stdout,"*********************************************************************************\n");
    fprintf(stdout,"Total payload size = %d\n",total_payload_size);
    fprintf(stdout,"Total padding size = %d\n",total_padding_size);
    fprintf(stdout,"Total free size = %d\n",total_free_size);
    fprintf(stdout,"Total used size = %d\n",total_used_size);
    fprintf(stdout,"*********************************************************************************\n");
    fflush(stdout);

    return;
}
