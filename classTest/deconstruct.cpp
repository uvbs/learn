/*************************************************************************
    > File Name: deconstruct.cpp
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Tue 10 Feb 2015 02:26:34 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#define SAFE_RELEASE(ptr)   \
    do {                    \
        if (ptr != NULL) {  \
            free(ptr);      \
            ptr = NULL;     \
        }                   \
    } while (0)


class One
{
public:
    One() {
        std::cout << "Instance" << std::endl;
        ptr = (char *)malloc(10);
        printf("ptr:%p\n", ptr);
    }

    ~One() {
        std::cout << "~Instance" << std::endl;
        SAFE_RELEASE(ptr);
    }
private:
    int a;
    char * ptr;
};

class Another
{
public:
    Another() {
        std::cout << "Another" << std::endl;
        ptr = (int *)malloc(10 * sizeof(int));
    }

    ~Another() {
        std::cout << "~Another" << std::endl;
        if (ptr) {
            std::cout << "ptr not null" << std::endl;
        }
        printf("ptr:%p\n", ptr);
        SAFE_RELEASE(ptr);
    }
private:
    int a;
    int b;
    int * ptr;
    int c;
};

int main()
{
    Another * ins = (Another *)(new One());
    delete ins;

    return 0;
}
