/*************************************************************************
    > File Name: info.cpp
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Sat 24 Jan 2015 05:50:19 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char * argv[])
{
    struct addrinfo hits;
    struct addrinfo *info, *p;

    hits.ai_family = AF_INET;
    hits.ai_socktype = SOCK_STREAM;
    hits.ai_flags = AI_PASSIVE;

    int ret = 0;
    if ((ret = getaddrinfo("127.0.0.1", "80", &hits, &info)) != 0) {
        printf("getaddr info error, %s\n", strerror(ret));
        return 0;
    }

    for (p = info; p != NULL; p = p->ai_next) {
        printf("p, %s\n", inet_ntoa(((struct sockaddr_in *)(p->ai_addr))->sin_addr)); 
    }
    return 0;
}
