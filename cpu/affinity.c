/*************************************************************************
    > File Name: affinity.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Wed 02 Dec 2015 03:32:21 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <linux/unistd.h>

/*
 * provide the proper syscall information if our libc
 * is not yet updated.
 */
#ifndef __NR_sched_setaffinity
#define __NR_sched_setaffinity  241
#define __NR_sched_getaffinity  242
_syscall3 (int, sched_setaffinity, pid_t, pid, unsigned int, len, unsigned long *, user_mask_ptr)
_syscall3 (int, sched_getaffinity, pid_t, pid, unsigned int, len, unsigned long *, user_mask_ptr)
#endif

int main(int argc, char * argv[])
{
    unsigned long new_mask = 2;
    unsigned int len = sizeof(new_mask);
    unsigned long cur_mask;
    pid_t p = 0;
    int ret;

    ret = sched_getaffinity(p, len, NULL);
    printf(" sched_getaffinity = %d, len = %u\n", ret, len);

    ret = sched_getaffinity(p, len, &cur_mask);
    printf(" sched_getaffinity = %d, cur_mask = %08lx\n", ret, cur_mask);

    //ret = sched_setaffinity(p, len, &new_mask);
    //printf(" sched_setaffinity = %d, new_mask = %08lx\n", ret, new_mask);

    ret = sched_getaffinity(p, len, &cur_mask);
    printf(" sched_getaffinity = %d, cur_mask = %08lx\n", ret, cur_mask);

    return 0;
}
