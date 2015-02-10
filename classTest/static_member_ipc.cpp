/*************************************************************************
    > File Name: static_member_ipc.cpp
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Tue 10 Feb 2015 10:45:36 AM CST
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>

class Listener
{
public:
    virtual ~Listener(){}
    virtual void onLog(std::string log) = 0;
};

class Log
{
public:
    static void setListener(Listener * _l) {
        l = _l;
    }

    static void verbose(std::string log) {
        if (l != NULL) {
            l->onLog(log);
        }
    }
    

private:
    static Listener * l;
};

Listener * Log::l = NULL;

class Instance : public Listener
{
public:
    virtual void onLog(std::string log) {
        printf("[p:%d][l:%p][%s]\n", getpid(), this, log.c_str());
    }
    
};

int main()
{
    Instance * ins = new Instance();
    Log::setListener(ins);

    int pid = fork();
    if (pid == 0) {
        Instance * another = new Instance();
        Log::setListener(another);
    }

    for (int i = 0; i < 100; i++) {
        char buf[10] = {0};
        snprintf(buf, 10, "now:%d", i);
        Log::verbose(buf);
        usleep(100000);
    }

    return 0;
}
