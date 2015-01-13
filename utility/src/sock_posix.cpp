/*************************************************************************
  > File Name: sock.cpp
  > Author: sudoku.huang
  > Mail: sudoku.huang@gmail.com 
  > Created Time: Tue 13 Jan 2015 04:57:40 PM CST
 ************************************************************************/

#include <socki.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>
#include <fcntl.h>

namespace sudoku
{
    namespace sock 
    {
        void closesocket(int * sockfd) 
        {
            if (*sockfd > 0) {
                ::close(*sockfd);
                *sockfd = -1;
            }
        }

        int socketpair(int family, int type, int protocol, int fd[2])
        {
            return ::socketpair(family, type, protocol, fd); 
        }

        int setnoblock(int sockfd)
        {
            int flags = fcntl(sockfd, F_GETFL, 0);
            if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
                return -1;
            }
            return 0;
        }

        void closepair(int fd[2])
        {
            closesocket(&fd[0]);
            closesocket(&fd[1]);
        }
    }
}
