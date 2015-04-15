#ifndef __HostSolver_H
#define __HostSolver_H

#include "tinythread.h"
#include <windows.h>
#include <map>
#include <set>
#include <string>
#include <vector>

/**
	用于异步解析dns
*/
class HostSolver
{
public:
	class Listener
	{
	public:
		enum Event{EVT_HOSTSOLVED, EVT_CANCELED, EVT_FAILED};
		virtual ~Listener() {}
		virtual void onSolverEvent(Event event, const char* host, HostSolver* solver) = 0;
	};

	HostSolver();
	virtual	~HostSolver();
	bool solve(const char* host);
	bool solve(const char* host, Listener* ln);
	void cancel(const char* host);
	void cancelAll();
	bool isSolved(const char* host) const;
	bool isPending(const char* host) const;
	bool getHostByName(std::vector<unsigned int>& out, const char* host) const;

private:
	bool start();
	void stop();
	void removePending(const char* host);
	void processMessages();
	void notifyListeners(Listener::Event event, const char* host);
	HWND createMessageOnlyWindow();
	std::string findHostByHandle(HANDLE h) const;

	static unsigned int	CALLBACK _solverThread(void* data);
	static LRESULT		CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	typedef struct 
	{
		HANDLE					hGetHost;
		std::set<Listener*>		listeners;
		char*					bufGetHost;
	}PendingData;

	typedef tthread::recursive_mutex							Mutex;
	typedef std::map<std::string, std::vector<unsigned int> >	SolvedMap;
	typedef std::map<std::string, PendingData>					PendingMap;
	typedef std::map<HANDLE, std::string>						Handle2HostMap;
	
	SolvedMap				mSolvedMap;
	PendingMap				mPendingMap;
	Handle2HostMap			mHandle2HostMap;
	HANDLE					mThread;
	unsigned int			mThreadId;
	HWND					mHwndMsg;
	bool					mIsRunning;
	HANDLE					mStartedEvent;
	mutable Mutex			mMutex;
};

#endif // __HostSolver_H