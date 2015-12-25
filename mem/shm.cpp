/*************************************************************************
    > File Name: shm.cpp
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Fri 11 Dec 2015 02:43:37 PM CST
 ************************************************************************/

#include<iostream>
using namespace std;

#include <sys/types.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/ipc.h>

#include <errno.h>
int main()
{
        int key = 0x1f123ca;
        int size = 4096;
        int shmid = shmget((key_t)key, size, IPC_CREAT | 0666);
        void * ptr = NULL;

        if(shmid == -1 )
        {
                perror("shmget error:");
                return 0;
        }
        else
        {
                char * addr = (char *)0x2f3850522000;
                printf("addr:%p, map:%p, errno:%d\n", addr, ptr, errno);
                ptr = shmat(shmid, (void *)addr, 0);
                printf("addr:%p, map:%p, errno:%d\n", addr, ptr, errno);
                addr[0] = '0';
                addr[3] = '3';
                addr[10] = 'a';

                printf("addr:%p, map:%p, errno:%d\n", addr, ptr, errno);
                printf("0:%c, 3:%c, 10:%c\n", addr[0], addr[3], addr[10]);

                getchar();
        }

        shmdt(ptr);
        return 0;
}
