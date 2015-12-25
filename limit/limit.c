/*************************************************************************
    > File Name: limit.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Wed 16 Dec 2015 04:34:15 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/resource.h>

int get_stacksize()
{
    struct rlimit old;
    int ret = 0;

    ret = getrlimit(RLIMIT_STACK, &old);
    if (ret < 0) {
        printf("(%d)getrlimit error(%d):%d\n", getpid(), errno, strerror(errno));
        return 0;
    }
    printf("(%d)RLIMIT_STACK, soft:%d, hard:%d\n", getpid(), old.rlim_cur, old.rlim_max);
    return ret;
}

int set_stacksize(int cur, int max)
{
    struct rlimit new;
    new.rlim_cur = cur;
    new.rlim_max = max;
    printf("(%d)%s:%d, cur:%d, max:%d\n", getpid(), __func__, __LINE__, cur, max);
    
    return setrlimit(RLIMIT_STACK, &new);
}

int main(int argc, char * argv[])
{
    int ret = 0;
    int status;

    get_stacksize();
    set_stacksize(1024 * 1024, -1);
    //get_stacksize();
    if ((ret = fork()) == 0) { //child
        get_stacksize();
        set_stacksize(1024 * 1024, -1);
        get_stacksize();
        //execlp("./another", NULL);
        printf("child exit(%d)\n", getpid());
        sleep(-1);
        return 0;
    } else if (ret > 0) {
        get_stacksize();
        printf("wait child exit(%d)\n", ret);
        waitpid(ret, &status, WEXITED);
        printf("parent exit(%d), %d\n", getpid(), status);
        sleep(-1);
    }

    return 0;
}
