/*************************************************************************
    > File Name: static.h
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Wed 28 Oct 2015 02:45:56 PM CST
 ************************************************************************/

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
static int do_print(const char * arg)
{
    printf("%s\n", arg);
    return 0;
}

#ifdef __cplusplus
}
#endif
