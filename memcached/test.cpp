/*************************************************************************
    > File Name: test.cpp
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Sat 17 Jan 2015 04:41:25 PM CST
 ************************************************************************/

/*
 * g++ test.cpp -lmemcached -std=c++0x
 */

#include <iostream>

#include "client.h"

int main(int argc, char * argv[])
{
    Client client;

    client.insert("sudoku", "sudoku.huang@gmail.com", 1000000);

    std::cout << client.get("sudoku") << std::endl;

    return 0;
}
