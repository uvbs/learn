//======================================================================================
#ifndef __SOX_SELECTOR_H_INCLUDE__
#define __SOX_SELECTOR_H_INCLUDE__
//======================================================================================
#include <memory>
#include "common.h"
#include "SocketBase.h"
#include "singleton.h"
#include "TimerHandler.h"
#include "utility.h"
#include "tinythread.h"
#include "MessageQueue.h"
#include "NetworkStatus.h"

#include "min_heap.h"

//======================================================================================

enum {
	SEL_NONE = 0, SEL_ALL = -1, SEL_READ = 1, SEL_WRITE = 2, SEL_RW = 3, // SEL_ERROR = 4,

	// notify only
	SEL_TIMEOUT = 8,

	// setup only, never notify
	// SEL_R_ONESHOT = 32, SEL_W_ONESHOT = 64, SEL_RW_ONESHOT = 96,
	SEL_CONNECTING = 128
};

//======================================================================================
class InternalLooper;
class SelectorEPoll
	: public Singleton<SelectorEPoll>
{
public:
	SelectorEPoll();
	~SelectorEPoll();
	bool Start();
	void Stop();
	void StopSync();
	void joinEpollThread();
	void Run();

	void CloseTimeout(time_t tNow);
	void TimerCheck(time_t tNow);

	void SetEvent(SocketBase* s, int remove, int add);
	void SetEvent(SocketBase * s,bool bRead,bool bWrite,bool bDel);
	int onReadSocket(SocketBase *pSocket);
	int onWriteSocket(SocketBase *pSocket);
	//void CloseSocket(SocketBase* s);

	void AddTimerHandler(TimerHandler *pHandler);
	void RemoveTimerHandler(TimerHandler *pHandler);

	void removeSocket(SocketBase* s); 

	bool isRunning() const;

	android::sp<yymobile::Looper> getLooper();
	
	bool isConnectedToInternet();

private:
	int setWREFdSet(fd_set* rdset, fd_set* wrset, fd_set* exceptset);

	// check : destroy in loop
	void record_removed(SocketBase * s) 
	{
		m_mutex.lock();
		m_removed.insert(s); 
		m_mutex.unlock();
	} 
	bool isRemoved(SocketBase *s)	
	{
		m_mutex.lock();
		bool rt= m_removed.find(s) != m_removed.end(); 
		m_mutex.unlock();
		return rt;
	}
	void clearRemoved() 
	{  
		m_mutex.lock();
		m_removed.clear();
		m_mutex.unlock();
	}
	bool checkHopAndGetTime();

public:
	static uint32_t m_iHaoMiao;
	static uint32_t m_iNow;

private:
	void timeout_correct(struct timeval * tv);
	int timeout_next(struct timeval ** tv);
	int timeout_loop(struct timeval ** tv);

	void timeout_process();

	void wakeup_thread();

	int gettime(struct timeval * tv);

protected:
	volatile bool m_bStop;
	volatile bool m_bRunning;
	std::set<SocketBase *> m_setSocket;
	std::set<TimerHandler*> m_setTimer;
	std::set<SocketBase *> m_removed;
	std::auto_ptr<tthread::thread> m_epollThread;
	android::sp<InternalLooper> m_looper;
	tthread::recursive_mutex m_mutex;
	NetworkStatus* mNetworkStatus;

public:
	struct timeval m_event_tv;
	struct timeval m_cache_tv;

private:
	yymobilesdk::heap::CMiniHeap<TimerHandler, TimerHandlerCompare> mHeap;
	int m_signal_pair[2];
};

//======================================================================================
#endif

