/*************************************************************************
    > File Name: static.cpp
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Wed 28 Oct 2015 02:44:42 PM CST
 ************************************************************************/

#include <iostream>
#include "static.h"

using namespace std;

typedef int (*func)(const char *);

int main(int argc, char * argv[])
{
//    do_print("asdfa");
    func f = do_print;
    printf("ada\n");
    f("adfsafasdf\n");
    return 0;
}
