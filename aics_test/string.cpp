/*************************************************************************
    > File Name: string.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Tue 03 Nov 2015 10:32:00 AM CST
 ************************************************************************/

#include<stdio.h>

#include <string>

int main(int argc, char * argv[])
{
    char buf[] = "module app 0\n";

    for ( char* p = buf; *p && *p != '\n'; )
    {
        for ( ; *p == ' ' || *p == '\t'; ++p );
        char* q = p;
        for ( ; *p && *p != ' ' && *p != '\t' && *p != '\n'; ++p );
        if ( p == q ) continue;
        std::string str(q, p - q);
        printf("%s\n", str.c_str());
    }
}
