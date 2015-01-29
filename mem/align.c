/*************************************************************************
  > File Name: align.c
  > Author: sudoku.huang
  > Mail: sudoku.huang@gmail.com 
  > Created Time: Wed 28 Jan 2015 05:49:53 PM CST
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#ifndef INTERNAL_SIZE_T  
#define INTERNAL_SIZE_T size_t  
#endif  
#define SIZE_SZ                (sizeof(INTERNAL_SIZE_T))  
#ifndef MALLOC_ALIGNMENT  
#define MALLOC_ALIGNMENT       (2 * SIZE_SZ)  
#endif  


struct malloc_chunk {  
    INTERNAL_SIZE_T      prev_size;  /* Size of previous chunk (if free).  */  
    INTERNAL_SIZE_T      size;       /* Size in bytes, including overhead. */  
    struct malloc_chunk* fd;         /* double links -- used only if free. */  
    struct malloc_chunk* bk;  
};  

/*
An allocated chunk looks like this:  
chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  
|             Size of previous chunk, if allocated            | |  
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  
|             Size of chunk, in bytes                       |M|P|  
mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  
|             User data starts here...                          .  
.                                                               .  
.             (malloc_usable_size() bytes)                      .  
.                                                               |  
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  
|             Size of chunk                                     |  
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  
*/

#define MALLOC_ALIGN_MASK      (MALLOC_ALIGNMENT - 1)  
#define MIN_CHUNK_SIZE        (sizeof(struct malloc_chunk))  
#define MINSIZE \
    (unsigned long)(((MIN_CHUNK_SIZE+MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK))  

#define request2size(req)   \
    (((req) + SIZE_SZ + MALLOC_ALIGN_MASK < MINSIZE) ?  \
     MINSIZE :   \
     ((req) + SIZE_SZ + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK)

int main()
{
    printf("sizeof(size_t)= %d\n", sizeof(size_t));
    printf("sizeof(void *)= %d\n", sizeof(void *));
    printf("sizeof(int)= %d\n", sizeof(int));
    printf("minsize = %d\n", MINSIZE);

    return 0;
}
