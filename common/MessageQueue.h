#ifndef __MessageQueue_H
#define __MessageQueue_H

#include <assert.h>
#include <list>
#include <set>
#include "refbase.h"
#include "common.h"

namespace yymobile
{
typedef tthread::recursive_mutex Mutex;

class MessageQueue;
class Looper;
class Handler;

class Message 
	: public android::RefBase
{
	friend class Handler;
	friend class MessageQueue;
public:	
	typedef void (*cb_cleartok_t)(Message& msg);
	int what;
	int arg1, arg2;
	void* token;
	android::sp<android::RefBase> token_ref;
	cb_cleartok_t cb_cleartok;
	Handler* target;
	int64_t getDeliverTime() const {return deliver_time;}
private:
	int64_t deliver_time;	
private:	
	Message() : what(0), arg1(0), arg2(0), token(NULL), cb_cleartok(NULL), target(NULL), deliver_time(0){}
	virtual ~Message() {if(cb_cleartok){cb_cleartok(*this);}}
};

template <typename TokenPtr>
void delete_token(Message& msg)
{
	if(msg.token) {
		delete (TokenPtr)(msg.token);
		msg.token = NULL;
	}
}


class Handler
{
public:
	Handler();
	Handler(android::sp<Looper>& looper);
	virtual ~Handler();
	bool sendEmptyMessage(int what);
	bool sendMessage(int what, void* token, Message::cb_cleartok_t cleartok = NULL);
	bool sendMessage(int what, android::sp<android::RefBase>& tokenRef);
	bool sendMessage(android::sp<Message>& msg);
	bool sendEmptyMessageDelay(int what, int64_t delayMs);
	bool sendMessageDelay(int what, void* token, int64_t delayMs, Message::cb_cleartok_t cleartok = NULL);
	bool sendMessageDelay(int what, android::sp<android::RefBase>& tokenRef, int64_t delayMs);
	bool sendMessageDelay(android::sp<Message>& msg, int64_t delayMs);

	/*
		移除所有what成员于what参数的值相同的消息
	*/
	bool removeMessages(int what);
	/*
		移除所有token成员与token参数的值相同的消息
		注意: token不能为NULL
	*/
	bool removeMessages(void* token);

	/*
		移除所有token_ref成员指向refbase的消息
		注意: refbase不能为NULL
	*/
	bool removeMessages(android::RefBase* refbase);

	/*
		移除所有消息
	*/
	bool removeMessages();
	bool isExists(int what) const;
	bool isExists(void* token) const;
	bool isExists(android::RefBase* token) const;
	bool isExists() const;
	android::sp<Message> makeMessage(int what, void* token, int arg1, int arg2, Message::cb_cleartok_t cleartok = NULL);
	android::sp<Message> makeMessage(int what, android::sp<android::RefBase>& tokenRef, int arg1, int arg2);
	android::sp<Message> makeMessage();
	android::sp<Looper> getMyLooper();

	virtual void handleMessage(MessageQueue* mq, android::sp<Message>& msg);
	
	static void destroyMainLooper();
	static android::sp<Looper> getMainLooper();

private:
	android::sp<Looper> mLooper;
};

class Looper
	: public android::RefBase
{
public:
	Looper();
	virtual ~Looper() {}
	virtual android::sp<MessageQueue> getMessageQueue();
	virtual void stop() {}
	virtual unsigned int getThreadId() const = 0;
private:
	android::sp<MessageQueue> mMq;
};

android::sp<Looper> createThreadLooper();


struct msg_less 
	: public std::binary_function<android::sp<Message>, android::sp<Message>, bool>
{
	bool operator()(const android::sp<Message>& _Left, const android::sp<Message>& _Right) const;
};


class MessageQueue
	: public android::RefBase
{
	friend class Handler;
public:
	typedef std::multiset<android::sp<Message>, msg_less > SortedListType;
	
	typedef bool (*fn_compare_t)(const Message* msg1, const Message* msg2);
	
	MessageQueue();
	virtual ~MessageQueue();

	bool addMessage(android::sp<Message>& msg);
	
	/*
		返回隔多少ms后处理下一条消息, 如果消息队列为null 则返回0
	*/
	int64_t deliverMessages(int64_t now = 0);
	bool isEmpty() const;
	void deleteRemovedMessages();

	int64_t timeoutNext();

	/*
		移除指定的消息,delayDelete指定是否要延迟删除消息对象,
		例如在token 对象的方法中调用removeMessage, 如果没有延迟删除会使
		token 对象立即被删除,这样很容易出问题,延迟删除对象会在looper
		下一次处理消息队列的时候删除消息对象
	*/
	bool removeMessages(const Message* msg, fn_compare_t compare, bool delayDelete);
	
	template <typename FnHandleMsg>
	bool forEachMsgs(FnHandleMsg& handler)
	{
		assert(handler);
		MutexScoped guard(mCs);
		
		SortedListType::const_iterator it;
		for(it=mMsgList.begin(); it!=mMsgList.end(); ++it) {
			if(!handler(this, it->get()))
				return false;
		}
		return true;
	}
		
private:
	SortedListType* getMsgList();
	yymobile::Mutex* getMutex();
	
	template <typename MsgList>
	static void clearMsgList(MsgList* msgList)
	{
		if(msgList && !msgList->empty()) {
			msgList->clear();
		}
	}
	
private:
	SortedListType mMsgList;
	std::list<android::sp<Message> > mRemovedMsgList;
	Mutex mCs;
};


}

#endif // __MessageQueue_H
