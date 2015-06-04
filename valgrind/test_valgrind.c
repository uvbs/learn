/*************************************************************************
    > File Name: test_valgrind.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Wed 03 Jun 2015 08:25:43 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>

void f(void)
{
    int * x = malloc(10 * sizeof(int));
    x[1] = 0;
    free(x);
    x = NULL;
}

int main(int argc, char * argv[])
{
    f();
    return 0;
}
