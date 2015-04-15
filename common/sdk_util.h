/*************************************************************************
    > File Name: sdk_util.h
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Mon 05 Jan 2015 06:22:48 PM CST
 ************************************************************************/

#ifndef _YYMOBILESDK_TIME_H__
#define _YYMOBILESDK_TIME_H__

#include <winsock.h>

namespace yymobilesdk 
{
	namespace time
	{
		struct timezone
		{
			int  tz_minuteswest; /* minutes W of Greenwich */
			int  tz_dsttime;     /* type of dst correction */
		};

#define yymobilesdk_timercmp(tvp, uvp, cmp)		\
	(((tvp)->tv_sec == (uvp)->tv_sec) ?			\
		((tvp)->tv_usec cmp (uvp)->tv_usec) :	\
		((tvp)->tv_sec cmp (uvp)->tv_sec))

#define yymobilesdk_timerisset(tvp)		((tvp)->tv_sec || (tvp)->tv_usec)

#define yymobilesdk_timeradd(tvp, uvp, vvp)					\
	do  {													\
		(vvp)->tv_sec  = (tvp)->tv_sec + (uvp)->tv_sec;		\
		(vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec;	\
		if ((vvp)->tv_usec >= 1000000) {					\
			(vvp)->tv_sec ++;								\
			(vvp)->tv_usec -= 1000000;						\
		}													\
	} while (0)
	
#define yymobilesdk_timersub(tvp, uvp, vvp)					\
	do  {													\
		(vvp)->tv_sec  = (tvp)->tv_sec - (uvp)->tv_sec;		\
		(vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec;	\
		if ((vvp)->tv_usec < 0) {							\
			(vvp)->tv_sec --;								\
			(vvp)->tv_usec += 1000000;						\
		}													\
	} while (0)

		int gettimeofday(struct timeval * tv, struct timezone * tz);

		void timer_clear(struct timeval * tv);

		void timerset(struct timeval * tv, signed long long mircosecond);

		int time_distance(struct timeval * a, struct timeval * b);
	}

	namespace sock 
	{
		void closesocket(int * sockfd);

		int socketpair(int family, int type, int protocol, int fd[2]);

		int setnoblock(int sockfd);

		void closepair(int fd[2]);
	}
}

#endif