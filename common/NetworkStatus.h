#ifndef __NetworkStatus_H
#define __NetworkStatus_H

#include <windows.h>
#include <set>
#include "tinythread.h"

class NetworkStatus
{
public:
	static const int INTERNET_UNKNOWN		= 0;
	static const int INTERNET_CONNECTED		= 1;
	static const int INTERNET_DISCONNECTED	= 2;

	class IStatusChangeListener
	{
	public:
		virtual ~IStatusChangeListener(){}
		virtual void onStatusChange(int curStatus, int oldStatus) = 0;
	};

private:
	typedef tthread::recursive_mutex Mutex;
	
	HANDLE					mThread;
	unsigned int			mThreadId;
	volatile bool			mIsRunning;
	volatile bool			mStop;
	volatile int			mCurrentStatus;
	HANDLE					mStartedEvent;
	HANDLE					mWsaStatusChanngeEvent;
	Mutex					mListenerMutex;
	std::set<IStatusChangeListener*> mListeners;

private:
	bool start();
	void stop();
	void monitorNetorkStatus();
	void notifyListeners(int curStatus, int oldStatus);

	static unsigned int CALLBACK networkStatusThread(void* data);

public:
	NetworkStatus();
	~NetworkStatus();
	int getCurrentStatus() const;
	bool addListener(IStatusChangeListener* ln);
	void removeListener(IStatusChangeListener* ln);
	void removeAllListeners();
};

#endif //__NetworkStatus_H