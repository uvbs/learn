/*************************************************************************
    > File Name: socketi.h
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Tue 13 Jan 2015 04:44:04 PM CST
 ************************************************************************/

namespace sudoku
{
    namespace sock
    {
        namespace sock 
        {
            void closesocket(int * sockfd);

            int socketpair(int family, int type, int protocol, int fd[2]);

            int setnoblock(int sockfd);

            void closepair(int fd[2]);
        }
    }
}
