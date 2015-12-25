/*************************************************************************
    > File Name: goodbye.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Sun 13 Dec 2015 01:42:06 PM CST
 ************************************************************************/

#include <stdio.h>

int goodbye_world(int a0, int a1, int a2, int a3, int a4, int a5, int a6 ) 
{ 
    int tmp0, tmp1, tmp2, tmp3, tmp4;
    tmp0 = a0; 
    tmp1 = a1; 
    tmp2 = a2; 
    tmp3 = a3; 
    tmp4 = a4; 
    tmp0 = tmp0 + tmp1 + tmp2 + tmp3 + tmp4 + a5 + a6;
    return tmp0; 
}

int byebye_world( ) 
{ 
    return goodbye_world(1,2,3,4,5,6,7); 
}
