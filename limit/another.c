/*************************************************************************
    > File Name: another.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Wed 16 Dec 2015 05:19:41 PM CST
 ************************************************************************/

#include <stdio.h>

int main(int argc, char * argv[])
{
    printf("Another call, pid:%d\n", getpid());
    sleep(-1);
    return 0;
}
