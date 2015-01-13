/*************************************************************************
  > File Name: timei.cpp
  > Author: sudoku.huang
  > Mail: sudoku.huang@gmail.com 
  > Created Time: Tue 13 Jan 2015 04:55:55 PM CST
 ************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
#include <time.h>

#include <sys/timeb.h>
#include <sys/time.h>
#ifdef __cplusplus
}
#endif

namespace sudoku
{
    namespace time
    {
        static int abs(int a) 
        {
            return a >= 0 ? a : -a;
        }
        
#if 0 && defined(__GNUC__) && __GNUC__ >= 3         /* gcc 3.0 or later */
        static int ftime_gettimeofday(struct timeval * tv, struct timezone * tz)
        {   
            struct timeb tb;

            if (tv == NULL) {
                return -1;
            }

            ftime(&tb);
            //printf("[%s:%u][s:%llu, ms:%u, timezone:%d, dsttime:%d]\n",__FUNCTION__, __LINE__,   \
            //  tb.time, tb.millitm, tb.timezone, tb.dstflag);

            tv->tv_sec  = (long)tb.time;
            tv->tv_usec = tb.millitm * 1000;

            if (tz != NULL) {
                //tz->tz_minuteswest  = tb.timezone;
                //tz->tz_dsttime = tb.dstflag;
            }

            return 0;
        }
#endif

#if 0
        static int monotonic_gettimeofday(struct timeval * tv, struct timezone * tz) 
        {
            struct timespec ts;

            if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0) {
                return -1;
            }

            tv->tv_sec = ts.tv_sec;
            tv->tv_usec = ts.tv_nsec / 1000L;

            return 0;
        }
#endif

        int gettimeofdayi(struct timeval * tv, struct timezone * tz)
        {
            gettimeofday(tv, tz);
        }

        void timer_clear(struct timeval * tv) 
        {
            tv->tv_sec = tv->tv_usec = 0L;
        }

        void timerset(struct timeval * tv, signed long long mircosecond) 
        {
            tv->tv_sec  = mircosecond / 1000000LL;
            tv->tv_usec = mircosecond % 1000000LL;
        }

        int time_distance(struct timeval * a, struct timeval * b)
        {
            return abs((a->tv_sec - b->tv_sec) * 1000L + (a->tv_usec - b->tv_usec) / 1000L);
        }
    }
}
