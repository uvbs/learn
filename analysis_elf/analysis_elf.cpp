/*************************************************************************
    > File Name: analysis_elf.cpp
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Sun 13 Dec 2015 09:55:28 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char * argv[])
{
    if (argc < 2) {
        printf("Input elf file to analysis!\n");
        exit(0);
    }

    int ret = -1;
    struct stat stat_buf;
    if (stat(argv[1], &stat_buf) < 0) {
        printf("stat error,  %s\n", strerror(errno));
        exit(-1);
    }
    
    if (!(stat_buf.st_mode & S_IFREG)) {
        printf("not a regular file. %d\n", stat_buf.st_mode);
        exit(-1);
    }
    printf("regluar file, %d\n", stat_buf.st_mode);
    
    int elf_fd = open(argv[1], O_RDONLY);
    if (elf_fd < 0) {
        printf("open %s failed, errno:%d, err:%s\n", argv[1], errno, strerror(errno));
        exit(-1);
    }

    close(elf_fd);
    elf_fd = -1;
    return 0;
}
