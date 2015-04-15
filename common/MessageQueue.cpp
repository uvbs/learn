#include "tinythread.h"
#include "MessageQueue.h"
#include "utility.h"
#define TAG "[MQ_HDR]"
#include "CLogger.h"
#include "selector_epoll.h"

using namespace android;
using namespace tthread;
using namespace yymobile;

#define USE_EPOLL_LOOPER 1

#ifdef __FLASHPLAYER
#define getTickInterval utility::flasccGetTickInterval
#else
#define getTickInterval() (30L)
#endif

Looper::Looper()
{
	mMq = sp<MessageQueue>(new MessageQueue);
}

sp<MessageQueue> Looper::getMessageQueue()
{
	return mMq;
}

class ThreadLooper
	: public Looper
{
public:
	ThreadLooper()
	{
		mStop = false;
		mThread = new thread(threadLoop, this);
	}

	virtual ~ThreadLooper()
	{
		stop();
		delete mThread;
	}

	virtual void stop()
	{
		if(!mStop) {
			mStop = true;
			if(mThread)
				mThread->join();
		}
	}

	unsigned int getThreadId() const
	{
		return mThread->native_id();
	}

	static void threadLoop(void* data)
	{	
		HRESULT hr = CoInitialize(0);
		if(hr != S_OK) {
			Log.w(TAG, "CoInitialize failed, hr=%u", hr);
		}

		static const int64_t TICK_INTERVAL = getTickInterval();
		ThreadLooper* thiz = (ThreadLooper*)data;
		while(!thiz->mStop) {
			android::sp<MessageQueue> mq = thiz->getMessageQueue();
			if(mq->isEmpty()) {
				this_thread::sleep_for(chrono::milliseconds(TICK_INTERVAL));
			} else {
				int64_t delay =	mq->deliverMessages();
				mq->deleteRemovedMessages();
				if(delay > TICK_INTERVAL)
					delay = TICK_INTERVAL;	
				this_thread::sleep_for(chrono::milliseconds(delay));
			}
		}

		CoUninitialize();
	}
private:
	thread* mThread;
	bool mStop;
};


android::sp<Looper> yymobile::createThreadLooper()
{
	return new ThreadLooper;
}

Handler::Handler()
{
	mLooper = getMainLooper();
}

Handler::Handler(android::sp<Looper>& looper)
{
	if(!looper.get())
		mLooper = getMainLooper();
	else
		mLooper = looper;
}

Handler::~Handler()
{
	removeMessages();
}

static sp<Looper> _gs_mainLooper;
android::sp<Looper> Handler::getMainLooper()
{
	if(!_gs_mainLooper.get()) {
#if USE_EPOLL_LOOPER
		_gs_mainLooper = sp<Looper>(SelectorEPoll::getInstance().getLooper());
#else
		_gs_mainLooper = sp<Looper>(new ThreadLooper);
#endif
	}
	return _gs_mainLooper;		
}

void Handler::destroyMainLooper()
{
	if(_gs_mainLooper.get()) {
		_gs_mainLooper->stop();
		_gs_mainLooper.clear();
	}
}

bool Handler::sendEmptyMessage(int what)
{
	return sendEmptyMessageDelay(what, 0);
}

bool Handler::sendMessage(int what, void* token, Message::cb_cleartok_t cleartok)
{
	sp<Message> msg = makeMessage(what, token, 0, 0, cleartok);
	return sendMessage(msg);
}

bool Handler::sendMessage(int what, sp<RefBase>& tokenRef)
{
	sp<Message> msg = makeMessage(what, tokenRef, 0, 0);
	return sendMessage(msg);
}

bool Handler::sendMessage(sp<Message>& msg)
{
	return sendMessageDelay(msg, 0);
}

bool Handler::sendEmptyMessageDelay(int what, int64_t delayMs)
{
	sp<Message> msg = makeMessage();
	msg->what = what;
	return sendMessageDelay(msg, delayMs);
}

bool Handler::sendMessageDelay(int what, void* token, int64_t delayMs, Message::cb_cleartok_t cleartok)
{
	sp<Message> msg = makeMessage(what, token, 0, 0, cleartok);
	return sendMessageDelay(msg, delayMs);
}

bool Handler::sendMessageDelay(int what, sp<RefBase>& tokenRef, int64_t delayMs)
{
	sp<Message> msg = makeMessage(what, tokenRef, 0, 0);
	return sendMessageDelay(msg, delayMs);
}

bool Handler::sendMessageDelay(sp<Message>& msg, int64_t delayMs)
{
	// yymobile::uptimeMillis() 增加的粒度比较大,+1 是为了处理这种情况:
	// 在handleMessage时调用sendMessage 再次把消息入队, 这时消息的deliverTimer 
	// 值还是当前时间,(yymobile::uptimeMillis()返回值可能还没有改变,  
	// 这样MessageQueue::deliverMessages 会继续处理这个消息而不是在下一个时间
	// 点在处理
	// 
	msg->deliver_time = (int64_t)getElapsedRealtime() + delayMs + 1;
	msg->target = this;
	return mLooper->getMessageQueue()->addMessage(msg);
}

bool Handler::isExists(int what) const
{
	MessageQueue::SortedListType* msgList = mLooper->getMessageQueue()->getMsgList();
	Mutex* cs = mLooper->getMessageQueue()->getMutex();
	assert(msgList && cs);
	
	MessageQueue::SortedListType::const_iterator it;
	cs->lock();
	for(it=msgList->begin(); it!=msgList->end(); ++it) {
		if((*it)->target == this && (*it)->what == what) {
			cs->unlock();
			return true;
		}
	}
	cs->unlock();
	return false;
}

bool Handler::isExists(void* token) const
{
	MessageQueue::SortedListType* msgList = mLooper->getMessageQueue()->getMsgList();
	Mutex* cs = mLooper->getMessageQueue()->getMutex();
	assert(msgList && cs);
	
	MessageQueue::SortedListType::const_iterator it;
	cs->lock();
	for(it=msgList->begin(); it!=msgList->end(); ++it) {
		if((*it)->target == this && (*it)->token == token) {
			cs->unlock();
			return true;
		}
	}
	cs->unlock();
	return false;
}

bool Handler::isExists(RefBase* refbase) const
{
	MessageQueue::SortedListType* msgList = mLooper->getMessageQueue()->getMsgList();
	Mutex* cs = mLooper->getMessageQueue()->getMutex();
	assert(msgList && cs);
	
	MessageQueue::SortedListType::const_iterator it;
	cs->lock();
	for(it=msgList->begin(); it!=msgList->end(); ++it) {
		if((*it)->target == this && (*it)->token_ref.get() == refbase) {
			cs->unlock();
			return true;
		}
	}
	cs->unlock();
	return false;
}

bool Handler::isExists() const
{
	MessageQueue::SortedListType* msgList = mLooper->getMessageQueue()->getMsgList();
	Mutex* cs = mLooper->getMessageQueue()->getMutex();
	assert(msgList && cs);
	
	MessageQueue::SortedListType::const_iterator it;
	cs->lock();
	for(it=msgList->begin(); it!=msgList->end(); ++it) {
		if((*it)->target == this) {
			cs->unlock();
			return true;
		}
	}
	cs->unlock();
	return false;
}

void Handler::handleMessage(MessageQueue* mq, android::sp<Message>& msg)
{
	Log.i(TAG, "no handler for msg %u", msg->what);
}

static bool compareToken(const Message* lhs, const Message* rhs)
{
	return (lhs->target == rhs->target) && (lhs->token == rhs->token);
}

static bool compareTokenRef(const Message* lhs, const Message* rhs)
{
	return (lhs->target == rhs->target) && (lhs->token_ref.get() == rhs->token_ref.get());
}

static bool compareWhat(const Message* lhs, const Message* rhs)
{
	return (lhs->target == rhs->target) && (lhs->what == rhs->what);
}

static bool compareTarget(const Message* lhs, const Message* rhs)
{
	return (lhs->target == rhs->target);
}

bool Handler::removeMessages(void* token)
{
	assert(token);
	sp<Message> msg = makeMessage();
	msg->token = token;
	return mLooper->getMessageQueue()->removeMessages(msg.get(), compareToken, true);
}

bool Handler::removeMessages(RefBase* refbase)
{
	assert(refbase);
	sp<Message> msg = makeMessage();
	msg->token_ref = refbase;
	return mLooper->getMessageQueue()->removeMessages(msg.get(), compareTokenRef, true);
}

bool Handler::removeMessages(int what)
{
	sp<Message> tmpMsg = makeMessage();
	tmpMsg->what = what;
	return mLooper->getMessageQueue()->removeMessages(tmpMsg.get(), compareWhat, true);
}

bool Handler::removeMessages()
{
	sp<Message> tmpMsg = makeMessage();
	return mLooper->getMessageQueue()->removeMessages(tmpMsg.get(), compareTarget, true);
}

sp<Message> Handler::makeMessage(int what, void* token, int arg1, int arg2, Message::cb_cleartok_t cleartok)
{
	Message* msg = new Message;
	assert(msg);
	msg->what = what;
	msg->token = token;
	msg->cb_cleartok = cleartok;
	msg->arg1 = arg1;
	msg->arg2 = arg2;
	msg->target = this;
	return sp<Message>(msg);
}

sp<Message> Handler::makeMessage(int what, sp<RefBase>& tokenRef, int arg1, int arg2)
{
	Message* msg = new Message;
	assert(msg);
	msg->what = what;
	msg->token = NULL;
	msg->cb_cleartok = NULL;
	msg->token_ref = tokenRef;
	msg->arg1 = arg1;
	msg->arg2 = arg2;
	msg->target = this;
	return sp<Message>(msg);
}

sp<Message> Handler::makeMessage()
{
	Message* msg = new Message;
	assert(msg);
	msg->target = this;
	return sp<Message>(msg);
}

sp<Looper> Handler::getMyLooper()
{
	return mLooper;
}


////////////////////////////////////////////////////////////
#define DEBUG_MessageQueue 0

bool msg_less::operator()(const sp<Message>& _Left, const sp<Message>& _Right) const
{
	return (_Right->getDeliverTime() - _Left->getDeliverTime()) > 0;
}

MessageQueue::MessageQueue()
{
}

MessageQueue::~MessageQueue()
{
	mCs.lock();
	clearMsgList(&mMsgList);
	clearMsgList(&mRemovedMsgList);
	mCs.unlock();
}

bool MessageQueue::addMessage(sp<Message>& msg) 
{
	if(!msg.get())
		return false;
	mCs.lock();
	mMsgList.insert(msg);
	mCs.unlock();
	return true;
}

bool MessageQueue::removeMessages(const Message* msg, fn_compare_t compare, bool delayDelete)
{
	lock_guard<Mutex> guard(mCs);

	if(mMsgList.empty())
		return false;

	assert(msg && compare);
	
	bool removed = false;
	SortedListType::iterator it = mMsgList.begin();
	while(it!=mMsgList.end()) {
		if(compare(msg, it->get())) {
			SortedListType::iterator tmpIt = it;
			++tmpIt;
			if(delayDelete)
				mRemovedMsgList.push_back(*it);
			mMsgList.erase(it);
			it = tmpIt;
			removed = true;
		} else {
			++it;
		}
	}
	return removed;
}

int64_t MessageQueue::deliverMessages(int64_t now)
{
	//FUNC_LOG_IF(DEBUG_MessageQueue);
	
	int64_t diff = 0;
	
	mCs.lock();

	int64_t curTime = now;
	if (curTime == 0) {
		curTime = (int64_t)getElapsedRealtime();
	}

	while(!mMsgList.empty()) {
		SortedListType::iterator it = mMsgList.begin();
		sp<Message> msg = *it;
		assert(msg.get() && msg->target);	
		diff = curTime - msg->deliver_time;
		if(diff >= 0L) {		
			mMsgList.erase(it);
			mCs.unlock();
			msg->target->handleMessage(this, msg);	
			mCs.lock();
			diff = 0;
		} else {	
			break;
		}
	}

	mCs.unlock();	
	
	return -diff;
}

bool MessageQueue::isEmpty() const
{
	lock_guard<Mutex> guard(((MessageQueue*)this)->mCs);

	if(mMsgList.empty())
		return true;
	return false;
}

void MessageQueue::deleteRemovedMessages()
{
	lock_guard<Mutex>  guard(((MessageQueue*)this)->mCs);
	clearMsgList(&mRemovedMsgList);
}

MessageQueue::SortedListType* MessageQueue::getMsgList()
{
	return &mMsgList;
}

yymobile::Mutex* MessageQueue::getMutex()
{
	return &mCs;
}



int64_t yymobile::MessageQueue::timeoutNext()
{
	tthread::lock_guard<tthread::recursive_mutex> gurad(mCs);
	SortedListType::iterator it = mMsgList.begin();
	if (it == mMsgList.end()) {
		return 0;
	} else {
		return (* it)->deliver_time;
	}
}