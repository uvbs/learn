/*************************************************************************
  > File Name: windows_time.c
  > Author: sudoku.huang
  > Mail: sudoku.huang@gmail.com 
  > Created Time: Wed 07 Jan 2015 12:17:12 PM CST
 ************************************************************************/

#include<stdio.h>
#include <Windows.h>
#include <time.h>
#include <sys/timeb.h>

#include "min_heap.h"

#define TAG "test_minheap"

class A : public TimerHandler
{
    public:
        virtual void Timer() {
            printf("%s:%u, next:%u\n", __FUNCTION__, __LINE__, m_nextTime);
        }

        A() {
            printf("%s:%u, 0x%08x\n", __FUNCTION__, __LINE__, (uint32_t)this);
        }

        virtual ~A() {
            printf("%s:%u, 0x%08x\n", __FUNCTION__, __LINE__, (uint32_t)this);
        }
};

struct timezone
{
    int  tz_minuteswest; /* minutes W of Greenwich */
    int  tz_dsttime;     /* type of dst correction */
};

int ftime_gettimeofday(struct timeval * tv, struct timezone * tz)
{   
    struct _timeb tb;

    if (tv == NULL) {
        return -1;
    }

    _ftime(&tb);

    //printf("%s:%u|m:%I64u, ms:%u, tz:%d, dst:%u\n", __FUNCTION__, __LINE__, tb.time, tb.millitm, tb.timezone, tb.dstflag);

    tv->tv_sec  = tb.time;
    tv->tv_usec = tb.millitm * 1000;

    if (tz != NULL) {
        tz->tz_minuteswest  = tb.timezone;
        tz->tz_dsttime = tb.dstflag;
    }

    return 0;
}

unsigned __int64 getdiffmillisecond(int begin, int end)
{
    unsigned __int64 diff = 0;
    unsigned __int64 millisecond_per_day = 24Ui64 /*hours*/ * 60Ui64 /* minutes */ * 60Ui64 /* seconds */ * 1000Ui64 /*millisecond */ * 1000Ui64 /*mircosecond */ /** 1000Ui64 nanosecond*/; 

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
    printf("nano second between 1601 - 1970: %I64u\n", millisecond_diff_1601_1970);

    value |= filetime.dwHighDateTime;
    value <<= 32;
    value |= filetime.dwLowDateTime;
    printf("nano second between 1601 - now:  %I64u\n", value);

    value /= 10Ui64;                        //change to mircosecond, FILETIME, 100nanosecond interval.
    value -= millisecond_diff_1601_1970;    //change to UTC 1970.1.


    tv->tv_sec  = (long)(value / 1000000Ui64);
    tv->tv_usec = (long)(value % 1000000Ui64);

    if (tz != NULL) {
        static int tzflag = 0;
        if (!tzflag) {
            _tzset();
            tzflag |= 1;
        }

        // Adjust for the timezone west of Greenwich
        tz->tz_minuteswest  = _timezone / 60;
        tz->tz_dsttime      = _daylight;
    }

    return 0;
}

void test_minheap()
{
    min_heap_t min_heap;
    min_heap_ctor(&min_heap);
    printf("heap size = %d\n", min_heap_size(&min_heap));

    struct timeval tv;
    struct timezone tz;
    ftime_gettimeofday(&tv, &tz);
    printf("%s:%u|m:%u, us:%u, tz:%d, dst:%u\n", __FUNCTION__, __LINE__, tv.tv_sec, tv.tv_usec, tz.tz_minuteswest, tz.tz_dsttime);
    filetime_gettimeofday(&tv, &tz);
    printf("%s:%u|m:%u, us:%u, tz:%d, dst:%u\n", __FUNCTION__, __LINE__, tv.tv_sec, tv.tv_usec, tz.tz_minuteswest, tz.tz_dsttime);

    getchar();
    return;

#if 1
    int value;
    while (scanf("%d", &value) && value) {

        A * th = new A();
        th->m_nextTime = value;

        min_heap_push(&min_heap, th);
    }

    printf("heap size = %d\n", min_heap_size(&min_heap));
    while (!min_heap_empty(&min_heap)) {
        TimerHandler * th = min_heap_pop(&min_heap);
        printf("get top value:%d\n", th->m_nextTime);
        delete th;
    }
#endif

    printf("press to exit....\n");
    getchar();
    getchar();
}
