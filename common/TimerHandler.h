#ifndef _TIMERHANDLER_HEADER_
#define _TIMERHANDLER_HEADER_

#include "CLogger.h"

#include "common.h"
#include "sdk_util.h"
#include "min_heap.h"

class TimerHandler : public yymobilesdk::heap::CMiniHeapElem {
public:
	TimerHandler() {
		yymobilesdk::time::timer_clear(&m_interval);
		yymobilesdk::time::timer_clear(&m_tv_timeout);
	}
	virtual ~TimerHandler() {
	}

	void setInterval(int interval, struct timeval * now) {
		yymobilesdk::time::timerset(&m_interval, interval * 1000Ui64);
		yymobilesdk_timeradd(now, &m_interval, &m_tv_timeout);
		printf("TimerHandler, setInterval:s:%u, us:%u\n", m_interval.tv_sec, m_interval.tv_usec);
	}

	void adjust(void * arg) {
		struct timeval * tv = (struct timeval *)arg;
		yymobilesdk_timersub(&m_tv_timeout, tv, &m_tv_timeout);
	}

	virtual void Timer() = 0;

	struct timeval m_interval;
	struct timeval m_tv_timeout;
};

class TimerHandlerCompare
{
public:
	int operator()(const TimerHandler * th1, const TimerHandler * th2) {
		return yymobilesdk_timercmp(&th1->m_tv_timeout, &th2->m_tv_timeout, >=);
	}
};
#endif

