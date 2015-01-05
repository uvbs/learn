/*************************************************************************
    > File Name: timer.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Sun 04 Jan 2015 06:21:03 PM CST
 ************************************************************************/

#include <stdio.h>
#include <signal.h>

#include <event.h>

void onTimer(int sock, short event, void * arg)
{
    static int count = 0;

    printf("count = %d\n", ++ count);
    if (count >= 10) {
        return;
    }

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    event_add((struct event *)arg, &tv);
}

static void onSignal(evutil_socket_t fd, short event, void * arg)
{
    struct event * signal = (struct event *)arg;
    printf("%s: got signal %d\n", __FUNCTION__, EVENT_SIGNAL(signal));

    static int called = 0;
    if (called >= 2) {
        event_del(signal);
    }
    called ++;
}

int main()
{
    struct event_base * base = event_base_new();

    struct event evTime;
    evtimer_set(&evTime, onTimer, &evTime);
    event_base_set(base, &evTime);

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    event_add(&evTime, &tv);

    struct event signal_int;
    event_assign(&signal_int, base, SIGINT, EV_SIGNAL | EV_PERSIST, onSignal, &signal_int);
    event_add(&signal_int, NULL);
    
    event_base_dispatch(base);
    event_base_free(base);

    return 0;
}
