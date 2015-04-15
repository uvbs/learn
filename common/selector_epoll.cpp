//======================================================================================
#include <errno.h>
#include <list>
#include <windows.h>
#include <Sensapi.h>
#include "selector_epoll.h"
#include "TimerHandler.h"
#include "CLogger.h"

using namespace std;
using namespace tthread;
using namespace android;
using namespace yymobile;

//======================================================================================
#define EPOLL_SIZE 100
#define EPOLL_TIMEOUT 1

static int pipe_err = 0;
uint32_t SelectorEPoll::m_iHaoMiao = 0;
uint32_t SelectorEPoll::m_iNow     = 0;

static void epoll_pipe_handler(int){
	pipe_err++;
}

static void epollThread(SelectorEPoll* selepoll) 
{
	selepoll->Run();
}

//======================================================================================
// InternalLooper
class InternalLooper : public yymobile::Looper
{
public:
	InternalLooper() : mStop(false)
	{
		log(Info, "%s", __FUNCTION__);
	}

	~InternalLooper()
	{
		stop();
		log(Info, "%s", __FUNCTION__);
	}

	virtual void stop()
	{
		mStop = true;
	}

	bool isRunning() const
	{
		return !mStop;
	}

	bool isMessageQueueEmpty() const
	{
		android::sp<MessageQueue> mq = const_cast<InternalLooper*>(this)->getMessageQueue();
		return mq->isEmpty();
	}

	int64_t timeoutNext() {
		android::sp<MessageQueue> mq = const_cast<InternalLooper*>(this)->getMessageQueue();
		return mq->timeoutNext();
	}

	void processMessages(int64_t now = 0)
	{
		if(!mStop) {
			android::sp<MessageQueue> mq = getMessageQueue();
			mq->deliverMessages();
			mq->deleteRemovedMessages();
		}
	}

	unsigned int getThreadId() const
	{
		throw std::runtime_error("getThreadId is not implements");
	}

private:
	bool mStop;
};

//======================================================================================
const static char * TAG = "SelectorEPoll";

SelectorEPoll::SelectorEPoll()
{
	m_bStop = true;
	m_bRunning = false;

	struct timeval stv;
	gettimeofday(&stv, NULL);
	m_iHaoMiao = stv.tv_sec * 1000 + stv.tv_usec/1000;
	m_iNow     = stv.tv_sec;

	m_looper = new InternalLooper;

	mNetworkStatus = NULL;
}
//======================================================================================
SelectorEPoll::~SelectorEPoll() 
{
	Stop();
	yymobilesdk::sock::closepair(m_signal_pair);
}
//======================================================================================
bool SelectorEPoll::Start()
{
	if(m_bRunning) {
		log(Error, "%s failed, already running\n", __FUNCTION__);
		return false;
	}

	lock_guard<recursive_mutex> lockGuard(m_mutex);

	struct timeval stv;
	gettimeofday(&stv, NULL);
	m_iNow = stv.tv_sec;
	m_iHaoMiao = stv.tv_sec * 1000 + stv.tv_usec/1000;

	m_bStop = false;
	m_epollThread = auto_ptr<thread>(new thread((void(*)(void *))epollThread, this));
	if(m_epollThread->get_id() == thread::id(NULL)) {
		log(Error, "%s create epoll thread failed\n", __FUNCTION__);
		return false;
	}

	if(!mNetworkStatus)
		mNetworkStatus = new NetworkStatus;
	
	return true;
}
//======================================================================================
void SelectorEPoll::Stop()
{
	m_looper->stop();

	m_bStop = true;
	if(m_epollThread.get())
		m_epollThread->join();

	if(mNetworkStatus) {
		delete mNetworkStatus;
		mNetworkStatus = NULL;
	}
}

//======================================================================================
void SelectorEPoll::StopSync() 
{
	m_looper->stop();

	m_bStop = true;
	joinEpollThread();
}

void SelectorEPoll::joinEpollThread() 
{
	if(m_epollThread.get()) {
		if(m_epollThread->joinable())
			m_epollThread->join();
	}
}

bool SelectorEPoll::isRunning() const
{
	return m_bRunning;
}

int SelectorEPoll::setWREFdSet(fd_set* rdset, fd_set* wrset, fd_set* exceptset)
{
	//logCat(Info, "SelectorEPoll:begin setWREFdSet");
	FD_ZERO(rdset);
	FD_ZERO(wrset);
	FD_ZERO(exceptset);

	int max_fd = -1;

	for(set<SocketBase*>::iterator it=m_setSocket.begin();it!=m_setSocket.end();)
	{
		SocketBase* s =  (*it++);
		if (s == NULL || (s->fd() == -1)) {
			continue;
		}

		if (max_fd < s->fd()) {
			max_fd = s->fd();
		}

		if (SEL_READ & s->m_selEvent) {
			//logCat(Info, "SelectorEPoll: setWREFdSet read fd:%u",s->fd());
			FD_SET(s->fd(), rdset);
			FD_SET(s->fd(), exceptset);
		}
		if (SEL_WRITE & s->m_selEvent) {
			//logCat(Info, "SelectorEPoll: setWREFdSet write fd:%u",s->fd());
			FD_SET(s->fd(), wrset);
			FD_SET(s->fd(), exceptset);
		}
	}

	return max_fd;
}

void SelectorEPoll::Run()
{
//  logCat(Info, "\nSelectorEPoll begin run\n");
	fd_set rdset, wrset, exceptset;
	struct timeval stv;

	m_bRunning = true;

	yymobilesdk::time::gettimeofday(&stv, NULL);
	m_iNow = stv.tv_sec;
	m_iHaoMiao = stv.tv_sec * 1000 + stv.tv_usec / 1000;

	int netChkTimes = 0;
	int max_fd = 0;

	m_signal_pair[0] = m_signal_pair[1] = -1;
	int ret = yymobilesdk::sock::socketpair(AF_INET, SOCK_STREAM, IPPROTO_IP, m_signal_pair);
	printf("socketpair ret = %d\n", ret);

	struct timeval tv, *tvp, tv_last, tv_loop, *tvp_loop;
	yymobilesdk::time::timer_clear(&m_cache_tv);
	yymobilesdk::time::timer_clear(&tv_last);

	while (!m_bStop)
	{
		timeout_correct(&tv);
		if (!yymobilesdk_timerisset(&tv_last)) {
			tv_last = tv;
		}
		m_iHaoMiao = tv.tv_sec * 1000 + tv.tv_usec / 1000;

		m_mutex.lock();
		if (yymobilesdk::time::time_distance(&tv_last, &tv) >= 1000) {
			CloseTimeout(m_iHaoMiao);
			tv_last = tv;
		}
		max_fd = setWREFdSet(&rdset, &wrset, &exceptset);
		m_mutex.unlock();

		if (m_signal_pair[1] > 0) {
			FD_SET(m_signal_pair[1], &rdset);
		}

		tvp = &tv;
		timeout_next(&tvp);

		tvp_loop = &tv_loop;
		timeout_loop(&tvp_loop);
		if (tvp_loop != NULL) {
			if (tvp == NULL || yymobilesdk_timercmp(tvp, tvp_loop, <)) {
				tvp = tvp_loop;
			} 
		}

		if (tvp == NULL) {
			Log.e(TAG, "tvp = NULL, rdset.count = %d", rdset.fd_count);
		} else {
			Log.e(TAG, "tvp, s:%u, us:%u, rdset.count = %d", tvp->tv_sec, tvp->tv_usec, rdset.fd_count);
		}

		/* update last old time */
		gettime(&m_event_tv);
		yymobilesdk::time::timer_clear(&m_cache_tv);
		
		int excepCount = 0;
		
		int iActive = select(max_fd, &rdset, &wrset, &exceptset, tvp);
		if (iActive < 0 )
		{
			if (EINTR == errno) {
				// 处理消息
				if(m_looper.get())
					m_looper->processMessages();
				continue;
			} else {
				//log(Debug, "epoll error: error code=%u", WSAGetLastError());
				if(!m_looper->isRunning() || m_looper->isMessageQueueEmpty())
					Sleep(30);
			}
		}

		if(m_bStop) {
			break ;
		}

		if (/*1 || */iActive > 0) {

			if (FD_ISSET(m_signal_pair[1], &rdset)) {
				static char buf[1024];
				recv(m_signal_pair[1], buf, 1024, 0);
				Log.v(TAG, "[Run][signal pair]");
			}

			m_mutex.lock();
			list<SocketBase*> tmpList;
			for(set<SocketBase*>::iterator it=m_setSocket.begin(); it!=m_setSocket.end(); ++it) {
				tmpList.push_back(*it);
			}
			m_mutex.unlock();

			for(list<SocketBase*>::iterator it= tmpList.begin();it!= tmpList.end();)
			{
				SocketBase* s =  *it++;
				// 有时有可能s指向的socketbase对象已经被delete,这时不应执行其他操作
				if (isRemoved(s)) {
	//				log(Info, "Maybe socketBase has deleted by onReadSocket,so check it again");
					continue;
				}

				if (s == NULL || (s->fd() == -1))
				{
					continue;
				}
				if (FD_ISSET(s->fd(), &exceptset)) {
					if (isRemoved(s)) 
					{
						log(Info, "Error happened on deleted socket.");
						continue;
					}
					
					++excepCount;

					// s->onError(); // 忽略oob造成的异常
					continue;
				}
				if (FD_ISSET(s->fd(), &rdset)) {
					log(Debug, "happened FD_ISSET rdset.");
					onReadSocket(s);
				}
				if (isRemoved(s)) {
	//				log(Info, "Maybe socketBase has deleted by onReadSocket,so check it again");
					continue;
				}

				if (FD_ISSET(s->fd(), &wrset)) {
	//				log(Info, "happened FD_ISSET wrset");
					onWriteSocket(s);
				}
	//			printf("SelectorEpoll fd %u\n",s->fd());
			}
		}
		clearRemoved();

		if(iActive == EPOLL_SIZE)
			log(Info, "epoll reach the max size:%u m_setSocket:%u.", 100, m_setSocket.size());

		gettime(&m_cache_tv);

		/* handle TimerHandler */
		timeout_process();

		// 处理消息
		if(m_looper.get())
			m_looper->processMessages(m_cache_tv.tv_sec * 1000 + m_cache_tv.tv_usec / 1000);

		// 如果网络不可用则sleep
		bool networkAlive = this->isConnectedToInternet();
		if(!networkAlive || iActive == 0 ||  iActive == -1 || excepCount == iActive) {
			if(iActive != 0 && netChkTimes++ > 100) {
				log(Error, "%s, networkAlive=%d, iActive=%d, excepCount=%d", __FUNCTION__, networkAlive, iActive, excepCount);
				netChkTimes = 0;
			}
			Sleep(10);
		}
	}
	//log(Info, "exit the main loop");

	m_bRunning = false;
}
//======================================================================================
void SelectorEPoll::CloseTimeout(time_t tNow)
{
	set<SocketBase*>::iterator itTmp = m_setSocket.end();
	for(set<SocketBase*>::iterator it=m_setSocket.begin();it!=m_setSocket.end();)
	{
		itTmp = it++;
		(*itTmp)->CloseTimeout(tNow);
	}
}
//======================================================================================
void SelectorEPoll::TimerCheck(time_t tNow)
{
	for(set<TimerHandler*>::iterator it=m_setTimer.begin();it!=m_setTimer.end();it++)
	{
		//(*it)->TimerCheck((uint32_t)tNow);
	}
}

//======================================================================================

#if 0
void SelectorEPoll::EPollCtl(int iMethod, int iSocket, epoll_event ev) 
{
	int ret = epoll_ctl(m_hEPoll, iMethod, iSocket, &ev);

	if (ret != 0)
	{
		switch (errno)
		{
		case EBADF:log(Info, "m_hEPoll or fd is not a valid file descriptor. iSocket: %u method: %u", iSocket, iMethod);return;
		case EEXIST:log(Info,"was EPOLL_CTL_ADD, and the supplied file descriptor fd is already in m_hEPoll.");return;
		case EINVAL:log(Info,"m_hEPoll is not an epoll file descriptor, or fd is the same as m_hEPoll, or the requested operation op is not supported by this interface.");return;
		case ENOENT:log(Info,"op was EPOLL_CTL_MOD or EPOLL_CTL_DEL, and fd is not in m_hEPoll.iSocket: %u method: %u", iSocket, iMethod);return;
		case ENOMEM:log(Info,"There was insufficient memory to handle the requested op control operation.");return;
		case EPERM:log(Info, "The target file fd does not support epoll.");return;
		}
	}
}
#endif
//======================================================================================
void SelectorEPoll::SetEvent(SocketBase* s, int remove, int add) 
{
	//  logCat(Info, "SelectorEPoll SetEvent remove:%u add:%u",remove, add);
	lock_guard<recursive_mutex> lockGuard(m_mutex);

	if(s == NULL) { //可能在等锁的时候s被释放！
		return;
	}

	if(m_setSocket.find(s) == m_setSocket.end())
	{
		m_setSocket.insert(s);
	}

	if ((SEL_READ & remove)
		|| (SEL_WRITE & remove)) {
			s->m_selEvent = (s->m_selEvent) & (~remove);
	}
	if ((SEL_READ & add)
		|| (SEL_WRITE & add)) {
			s->m_selEvent = (s->m_selEvent | add);
	}
	wakeup_thread();
//	logCat(Info, "SelectorEPoll: SetEvent fd:%u",s->fd());
}
//======================================================================================
int SelectorEPoll::onReadSocket(SocketBase *pSocket)
{
	if(pSocket == NULL/* || pSocket->m_iSocket == -1*/)
		return 0;

	if (isRemoved(pSocket)) 
	{
		log(Info, "DESTROY IN LOOP FOUND.");
		return 0;
	}

	//log(Info, "onReadSocket,sockid:%u sockaddr:%u ", pSocket->m_iSocket, (uint32_t)pSocket);
	return pSocket->onReadSocket();
}
//======================================================================================
int SelectorEPoll::onWriteSocket(SocketBase *pSocket)
{
	if(pSocket == NULL/* || pSocket->m_iSocket == -1 || pSocket->m_iSocket == 0*/)
	{
		log(Warn, "onWriteSocket socket has close, pSocket==NULL");
		return 0;
	}

	if (isRemoved(pSocket))
	{
		log(Info, "onWriteSocket DESTROY IN LOOP FOUND" );
		return 0;
	}

	return pSocket->onWriteSocket();
}
//======================================================================================
void SelectorEPoll::removeSocket(SocketBase* s) 
{
	//log(Info, "removeSocket,sockid:%u sockaddr:%u ", s->m_iSocket, (uint32_t)s);

	lock_guard<recursive_mutex> lockGuard(m_mutex);

	record_removed(s);

	std::set<SocketBase *>::iterator it = m_setSocket.find(s);
	if(it == m_setSocket.end())
	{
		log(Error, "epoll removeSocket but not find socket id: %u", s->m_iSocket);
		return;
	}
	m_setSocket.erase(it);
}

//======================================================================================
sp<Looper> SelectorEPoll::getLooper() 
{
	return m_looper;
}

//======================================================================================
void SelectorEPoll::AddTimerHandler(TimerHandler *pHandler)
{
	lock_guard<recursive_mutex> lockGuard(m_mutex);
#if 0
	if(m_setTimer.find(pHandler)!=m_setTimer.end())
		return;

	m_setTimer.insert(pHandler);
#else
	Log.v(TAG, "AddTimerHandler[0x%08x]", pHandler);
	if (pHandler->m_min_heap_idx == -1) {
		mHeap.push(pHandler);
	} else {
		mHeap.adjust(pHandler);
	}
	wakeup_thread();
#endif
}

//======================================================================================
void SelectorEPoll::RemoveTimerHandler(TimerHandler *pHandler)
{
	lock_guard<recursive_mutex> lockGuard(m_mutex);
#if 0
	std::set<TimerHandler*>::iterator it = m_setTimer.find(pHandler);
	if(it != m_setTimer.end())
		m_setTimer.erase(it);
#else
	Log.v(TAG, "RemoveTimerHandler[0x%08x]", pHandler);
	mHeap.erase(pHandler);
#endif
}

bool SelectorEPoll::checkHopAndGetTime()
{
	static bool first = false;
	static struct timeval nowTimestamp;
	static struct timeval lastRedress;
	static struct timeval stv;
	static uint32_t lastInterval = 0;
	static uint32_t interval_msec = 0;

	if (first) {
		gettimeofday(&nowTimestamp, NULL);
		lastRedress = nowTimestamp;
		first = false;
	}

	gettimeofday(&nowTimestamp, NULL);
	uint64_t stopUs = ((uint64_t)nowTimestamp.tv_sec * 1000000) + nowTimestamp.tv_usec;
	uint64_t startUs = ((uint64_t)lastRedress.tv_sec * 1000000) + lastRedress.tv_usec;
	interval_msec =
		stopUs < startUs ? 0 : (uint32_t) ((stopUs - startUs) / 1000);
	/* end change by duzf */

	m_iHaoMiao += interval_msec - lastInterval;
	lastInterval = interval_msec;

	//Log.e("SelectorEPoll", "checkHopAndGetTime m_iHaoMiao %u", m_iHaoMiao);
	if (interval_msec > 1000) {
		gettimeofday(&stv, NULL);
		uint32_t nowHaoMiao = stv.tv_sec * 1000 + stv.tv_usec / 1000;
		m_iHaoMiao = nowHaoMiao;
		m_iNow = stv.tv_sec;
		lastRedress = nowTimestamp;
		lastInterval = 0;
		return true;
	}
	return false;
}

bool SelectorEPoll::isConnectedToInternet()
{
	if(!mNetworkStatus) {
		DWORD d = 0;
		BOOL rt = IsNetworkAlive(&d);
		int err = WSAGetLastError();
		if(err != 0) {
			log(Error, "%s IsNetworkAlive failed, err=%d", __FUNCTION__, err); // System Event Notification有可能挂了,返回true
			return true;
		}
		return rt ? true : false;
	}

	int curStatus = mNetworkStatus->getCurrentStatus();
	if(curStatus == NetworkStatus::INTERNET_UNKNOWN) {
		DWORD d = 0;
		BOOL rt = IsNetworkAlive(&d);
		int err = WSAGetLastError();
		if(err != 0) {
			log(Error, "%s IsNetworkAlive failed 2, err=%d", __FUNCTION__, err); // System Event Notification有可能挂了,返回true
			return true;
		}
		return rt ? true : false;
	}

	return (curStatus == NetworkStatus::INTERNET_CONNECTED) ? true : false;
}

void SelectorEPoll::timeout_correct(struct timeval * tv)
{
	struct timeval off;

	gettime(tv);

	if (yymobilesdk_timercmp(tv, &m_event_tv, >=)) {
		m_event_tv = *tv;
		return;
	}

	yymobilesdk_timersub(&m_event_tv, tv, &off);

	mHeap.doall(&TimerHandler::adjust, &off);
}

int SelectorEPoll::gettime(struct timeval * tv)
{
	if (m_cache_tv.tv_sec != 0) {
		*tv = m_cache_tv;
		return 0;
	}

	return yymobilesdk::time::gettimeofday(tv, NULL);
}

int SelectorEPoll::timeout_next(struct timeval ** tv)
{
	struct timeval now;
	TimerHandler * th = NULL;
	struct timeval * tvp = * tv;
	int ret = 0;

	th = mHeap.top();
	if (th == NULL) {
		*tv = NULL;
		return 0;
	}

	if (gettime(&now) == -1) {
		return -1;
	}

	if (yymobilesdk_timercmp(&th->m_tv_timeout, &now, <=)) {
		yymobilesdk::time::timer_clear(tvp);
		return 0;
	}
	yymobilesdk_timersub(&th->m_tv_timeout, &now, tvp);
	return 0;
}

int SelectorEPoll::timeout_loop(struct timeval ** tv)
{
	struct timeval now, next;
	struct timeval * tvp = * tv;
	int ret = 0;

	if (m_looper->isMessageQueueEmpty()) {
		*tv = NULL;
		return 0;
	}

	if (gettime(&now) == -1) {
		return -1;
	}

	int64_t timeoutNext = m_looper->timeoutNext();
	yymobilesdk::time::timerset(&next, timeoutNext * 1000L);

	if (yymobilesdk_timercmp(&next, &now, <=)) {
		yymobilesdk::time::timerset(tvp, 0);
	} else {
		yymobilesdk_timersub(&next, &now, tvp);
	}

	return 0;
}

void SelectorEPoll::timeout_process()
{
	tthread::lock_guard<tthread::recursive_mutex> guard(m_mutex);

	struct timeval now;
	TimerHandler * th = NULL;

	if (mHeap.empty()) {
		return;
	}

	gettime(&now);

	while ((th = mHeap.top()) != NULL) {
		printf("timeout_processs:%u, us:%u, now:%u,us:%u\n", th->m_tv_timeout.tv_sec, th->m_tv_timeout.tv_usec, now.tv_sec, now.tv_usec);
		if (yymobilesdk_timercmp(&th->m_tv_timeout, &now, >)) {
			break;
		}
		mHeap.pop();
		Log.v(TAG, "HandleTimerHandler[0x%08x]", th);

		th->Timer();
	}
}

void SelectorEPoll::wakeup_thread()
{
	static char msg = 1;
	send(m_signal_pair[0], &msg, 1, 0);
}
//======================================================================================
//======================================================================================
//end

