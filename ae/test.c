/*************************************************************************
    > File Name: test.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Thu 05 Feb 2015 09:41:54 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

#include "ae.h"

#define MAX_FD 5

void loop_init(struct aeEventLoop * l)
{
    puts("I'm loop_init");
    return;
}

void file_cb(struct aeEventLoop * l, int fd, void * data, int mask)
{
    char buf[51] = {0};
    ssize_t size = read(fd, buf, 51);
    buf[size] = 0;
    printf("I'm file_cb, [eventloop:%p, fd:%d, data:%p, mask:%d]\n",
        l, fd, data, mask);
    printf("file_cb, read:%s\n", buf);
    return;
}

int time_cb(struct aeEventLoop * l, long long id, void * data)
{
    printf("[time_cb][now is %ld\n", time(NULL));
    printf("I'm timecb, [eventloop:%p, id:%lld, data:%p]\n",
        l, id, data);
    return 5 * 1000;
}

void fin_cb(struct aeEventLoop * loop, void * data)
{
    printf("call the unknown final function\n");
    return;
}

int main(int argc, char * argv[])
{
    aeEventLoop * l;
    char * msg = "here std say!";
    char * userdata = (char *)malloc(sizeof(char) * 50);
    printf("userdata:%p\n", userdata);

    memset(userdata, 0, 50);
    memcpy(userdata, msg, sizeof(msg));

    l = aeCreateEventLoop(MAX_FD);
    aeSetBeforeSleepProc(l, loop_init);
    aeCreateFileEvent(l, STDIN_FILENO, AE_READABLE, file_cb, userdata);
    aeCreateTimeEvent(l, 5 * 1000, time_cb, NULL, fin_cb);

    aeMain(l);

    printf("end!!\n");

    return 0;
}
