/*************************************************************************
  > File Name: timei.h
  > Author: sudoku.huang
  > Mail: sudoku.huang@gmail.com 
  > Created Time: Tue 13 Jan 2015 04:42:36 PM CST
 ************************************************************************/

#ifndef _SUDOKU_TIME_H__
#define _SUDOKU_TIME_H_

#if defined(_WIN32) || defined(WIN32)
#include <winsock.h>
#else
#include <time.h>
#endif

namespace sudoku
{
    namespace time
    {
#if defined(_WIN32) || defined(WIN32)
        struct timezone
        {
            int  tz_minuteswest; /* minutes W of Greenwich */
            int  tz_dsttime;     /* type of dst correction */
        };
#endif

#define yymobilesdk_timercmp(tvp, uvp, cmp)     \
        (((tvp)->tv_sec == (uvp)->tv_sec) ?         \
         ((tvp)->tv_usec cmp (uvp)->tv_usec) :   \
         ((tvp)->tv_sec cmp (uvp)->tv_sec))

#define yymobilesdk_timerisset(tvp)     ((tvp)->tv_sec || (tvp)->tv_usec)

#define yymobilesdk_timeradd(tvp, uvp, vvp)                 \
        do  {                                                   \
            (vvp)->tv_sec  = (tvp)->tv_sec + (uvp)->tv_sec;     \
            (vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec;   \
            if ((vvp)->tv_usec >= 1000000) {                    \
                (vvp)->tv_sec ++;                               \
                (vvp)->tv_usec -= 1000000;                      \
            }                                                   \
        } while (0)

#define yymobilesdk_timersub(tvp, uvp, vvp)                 \
        do  {                                                   \
            (vvp)->tv_sec  = (tvp)->tv_sec - (uvp)->tv_sec;     \
            (vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec;   \
            if ((vvp)->tv_usec < 0) {                           \
                (vvp)->tv_sec --;                               \
                (vvp)->tv_usec += 1000000;                      \
            }                                                   \
        } while (0)

        int gettimeofdayi(struct timeval * tv, struct timezone * tz);

        void timer_clear(struct timeval * tv);

        void timerset(struct timeval * tv, signed long long mircosecond);

        int time_distance(struct timeval * a, struct timeval * b);
    }
}

#endif
