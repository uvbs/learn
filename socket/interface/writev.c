/*************************************************************************
    > File Name: writev.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Sat 17 Jan 2015 11:50:33 AM CST
 ************************************************************************/

#include <stdio.h>
#include <string.h>

#include <sys/uio.h>

int main(int argc, char * argv[])
{
    char * a = "hello";
    char * b = "world";
    char * c = "just test";

    struct iovec iov[3];
    iov[0].iov_base = a;
    iov[0].iov_len = strlen(a);
    iov[1].iov_base = b;
    iov[1].iov_len = strlen(b);
    iov[2].iov_base = c;
    iov[2].iov_len = strlen(c);

    writev(1, iov, 3);

    return 0;
}
