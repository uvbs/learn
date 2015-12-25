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

typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

typedef __signed__ long __s64;
typedef unsigned long __u64;

typedef __u64   Elf64_Addr;
typedef __u16   Elf64_Half;
typedef __s16   Elf64_SHalf;
typedef __u64   Elf64_Off;
typedef __s32   Elf64_Sword;
typedef __u32   Elf64_Word;
typedef __u64   Elf64_Xword;
typedef __s64   Elf64_Sxword;

#define EI_NIDENT 16

struct elf_header
{
    unsigned char e_ident[EI_NIDENT]; /* ELF "magic number" */
    Elf64_Half e_type;
    Elf64_Half e_machine;
    Elf64_Word e_version;
    Elf64_Addr e_entry;       /* Entry point virtual address */
    Elf64_Off e_phoff;        /* Program header table file offset */
    Elf64_Off e_shoff;        /* Section header table file offset */
    Elf64_Word e_flags;
    Elf64_Half e_ehsize;
    Elf64_Half e_phentsize;
    Elf64_Half e_phnum;
    Elf64_Half e_shentsize;
    Elf64_Half e_shnum;
    Elf64_Half e_shstrndx; 
};

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
