/*************************************************************************
    > File Name: sdk_util.cpp
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Mon 05 Jan 2015 06:22:48 PM CST
 ************************************************************************/

#include "sdk_util.h"

#include <time.h>
#include <sys/timeb.h>
#include <stdio.h>
#include <WinSock.h>

namespace yymobilesdk 
{
	namespace time
	{
		int ftime_gettimeofday(struct timeval * tv, struct timezone * tz)
		{	
			struct _timeb tb;

			if (tv == NULL) {
				return -1;
			}

			_ftime(&tb);
			//printf("[%s:%u][s:%I64u, ms:%u, timezone:%d, dsttime:%d]\n",__FUNCTION__, __LINE__,	\
			//	tb.time, tb.millitm, tb.timezone, tb.dstflag);

			tv->tv_sec	= (long)tb.time;
			tv->tv_usec = tb.millitm * 1000;

			if (tz != NULL) {
				tz->tz_minuteswest	= tb.timezone;
				tz->tz_dsttime = tb.dstflag;
			}

			return 0;
		}

		static unsigned __int64 getdiffmillisecond(int begin, int end)
		{
			unsigned __int64 diff = 0;
			unsigned __int64 millisecond_per_day = 24Ui64 /*hours*/ * 60Ui64 /* minutes */ * 60Ui64 /* seconds */ \
								* 1000Ui64 /*millisecond */ * 1000Ui64 /*mircosecond */ /** 1000Ui64 nanosecond*/; 

			for (int i = begin; i < end; i++) {
				if ((i % 4 == 0 && i % 100 != 0) || (i % 400 == 0)) { //Leap year
					diff += 366Ui64/*days */ * millisecond_per_day;
				} else {
					diff += 365Ui64/*days */ * millisecond_per_day;
				}
			}
			return diff;
		}

		int filetime_gettimeofday(struct timeval * tv, struct timezone * tz) 
		{
			FILETIME filetime;

			if (tv == NULL) {
				return -1;
			}

			GetSystemTimeAsFileTime(&filetime);
			unsigned __int64 value = 0;
			const static unsigned __int64 millisecond_diff_1601_1970 = getdiffmillisecond(1601, 1970);
			//printf("nano second between 1601 - 1970: %I64u\n", millisecond_diff_1601_1970);

			value |= filetime.dwHighDateTime;
			value <<= 32;
			value |= filetime.dwLowDateTime;
			//printf("nano second between 1601 - now:  %I64u\n", value);

			value /= 10Ui64;						//change to mircosecond, FILETIME, 100nanosecond interval.
			value -= millisecond_diff_1601_1970;	//change to UTC 1970.1.


			tv->tv_sec	= (long)(value / 1000000Ui64);
			tv->tv_usec = (long)(value % 1000000Ui64);

			if (tz != NULL) {
				static int tzflag = 0;
				if (!tzflag) {
					_tzset();
					tzflag |= 1;
				}

				// Adjust for the timezone west of Greenwich
				tz->tz_minuteswest  = _timezone / 60;
				tz->tz_dsttime		= _daylight;
			}

			return 0;
		}

		int gettimeofday(struct timeval * tv, struct timezone * tz)
		{
#if defined(USE_FILETIME)
			return filetime_gettimeofday(tv, tz);
#else
			return ftime_gettimeofday(tv, tz);
#endif
		}

		void timer_clear(struct timeval * tv) 
		{
			tv->tv_sec = tv->tv_usec = 0;
		}

		void timerset(struct timeval * tv, signed long long mircosecond) 
		{
			tv->tv_sec  = mircosecond / 1000000;
			tv->tv_usec = mircosecond % 1000000;
		}

		int time_distance(struct timeval * a, struct timeval * b)
		{
			return abs((a->tv_sec - b->tv_sec) * 1000 + (a->tv_usec - b->tv_usec) / 1000);
		}
	}

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

			WSADATA	wd;
			int result = WSAStartup(MAKEWORD(1, 1), &wd);

			listener = socket(AF_INET, type, IPPROTO_IP);
			if (listener < 0) {
				printf("%s:%u\n", __FUNCTION__, __LINE__);
				return -1;
			}

			memset(&listen_addr, 0, sizeof(listen_addr));
			listen_addr.sin_family		= AF_INET;
			listen_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
			listen_addr.sin_port		= 0; /* random port by system */
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