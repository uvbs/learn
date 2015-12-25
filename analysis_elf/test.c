/*************************************************************************
    > File Name: test.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Sun 13 Dec 2015 09:22:27 AM CST
 ************************************************************************/

#include <stdio.h>

char * str = "sudoku.huang";
static char * str1 = "huangkun.kun";
int a = 10;
int b;
char * ptr;

int main(int argc, char * argv[])
{
    printf("main:%p\n", main);
    printf("%s:%p:%d\n", str, str, getpid());
    getchar();
 
    return 0;
}
