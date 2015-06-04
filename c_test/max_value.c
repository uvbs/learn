/*************************************************************************
    > File Name: max_value.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Fri 13 Feb 2015 04:02:22 PM CST
 ************************************************************************/

#include<stdio.h>

int main()
{
    int value = (~(1 << 31) & 0xFFFFFFFF - 1);

    printf("%d, %u\n", value);

    short min_int16 = 0x8000;
    printf("min_int16 = %d\n", min_int16);


    short max_int16 = 0x7fff;
    printf("max_value = %d\n", max_int16);

    return 0;
}
