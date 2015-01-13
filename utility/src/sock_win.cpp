/*************************************************************************
  > File Name: sock.cpp
  > Author: sudoku.huang
  > Mail: sudoku.huang@gmail.com 
  > Created Time: Tue 13 Jan 2015 04:57:40 PM CST
 ************************************************************************/

namespace sudoku
{
    namespace sock 
    {
        typedef int socklen_t;
        void closesocket(int * sockfd) 
        {
            if (*sockfd > 0) {
                ::closesocket(*sockfd);
                *sockfd = -1;
            }
        }

        int socketpair(int family, int type, int protocol, int fd[2])
        {
            int listener  = -1;
            int connector = -1;
            int acceptor  = -1;

            struct sockaddr_in listen_addr;
            struct sockaddr_in connect_addr;
            socklen_t size;

            int save_errno = -1;

            if (protocol || family != AF_INET) {
                printf("%s:%u\n", __FUNCTION__, __LINE__);
                return -1;
            }

            if (fd == NULL) {
                printf("%s:%u\n", __FUNCTION__, __LINE__);
                return -1;
            }

            WSADATA wd;
            int result = WSAStartup(MAKEWORD(1, 1), &wd);

            listener = socket(AF_INET, type, IPPROTO_IP);
            if (listener < 0) {
                printf("%s:%u\n", __FUNCTION__, __LINE__);
                return -1;
            }

            memset(&listen_addr, 0, sizeof(listen_addr));
            listen_addr.sin_family      = AF_INET;
            listen_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            listen_addr.sin_port        = 0; /* random port by system */
            if (bind(listener, (struct sockaddr *) &listen_addr, sizeof(listen_addr)) == -1) {
                printf("%s:%u\n", __FUNCTION__, __LINE__);
                goto tidy_up_and_failed;
            }
            if (listen(listener, 1) == -1) {
                printf("%s:%u\n", __FUNCTION__, __LINE__);
                goto tidy_up_and_failed;
            }

            connector = socket(AF_INET, type, IPPROTO_IP);
            if (connector < 0) {
                printf("%s:%u\n", __FUNCTION__, __LINE__);
                goto tidy_up_and_failed;
            }

            size = sizeof(connect_addr);
            memset(&connect_addr, 0, sizeof(connect_addr));
            if (getsockname(listener, (struct sockaddr *) &connect_addr, &size) == -1) {
                printf("%s:%u\n", __FUNCTION__, __LINE__);
                goto tidy_up_and_failed;
            }
            if (size != sizeof(connect_addr)) {
                goto tidy_up_and_failed;
            }
            if (connect(connector, (struct sockaddr *) &connect_addr, sizeof(connect_addr)) < 0) {
                printf("%s:%u\n", __FUNCTION__, __LINE__);
                goto tidy_up_and_failed;
            }

            size = sizeof(listen_addr);
            memset(&listen_addr, 0, sizeof(listen_addr));
            acceptor = accept(listener, (struct sockaddr *) &listen_addr, &size);
            if (acceptor < 0) {
                printf("%s:%u\n", __FUNCTION__, __LINE__);
                goto tidy_up_and_failed;
            }
            if (size != sizeof(listen_addr)) {
                printf("%s:%u\n", __FUNCTION__, __LINE__);
                goto tidy_up_and_failed;
            }

            closesocket(&listener);

            memset(&connect_addr, 0, sizeof(connect_addr));
            if (getsockname(connector, (struct sockaddr *) &connect_addr, &size) < 0) {
                printf("%s:%u\n", __FUNCTION__, __LINE__);
                goto tidy_up_and_failed;
            }

            /* check */
            if (size != sizeof(connect_addr) ||
                    listen_addr.sin_family != connect_addr.sin_family ||
                    listen_addr.sin_addr.s_addr != connect_addr.sin_addr.s_addr ||
                    listen_addr.sin_port != connect_addr.sin_port) {
                printf("%s:%u\n", __FUNCTION__, __LINE__);
                goto tidy_up_and_failed;
            }

            fd[0] = connector;
            fd[1] = acceptor;

            return 0;

tidy_up_and_failed:
            closesocket(&listener);
            closesocket(&connector);
            closesocket(&acceptor);
            return -1;
        }

        int setnoblock(int sockfd)
        {
            u_long nonblocking = 1;
            if (ioctlsocket(sockfd, FIONBIO, &nonblocking) == SOCKET_ERROR) {
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
