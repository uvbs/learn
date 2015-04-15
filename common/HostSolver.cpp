#include <winsock2.h>
#include "HostSolver.h"
#include <assert.h>
#include <process.h>

#define WM_HOSTRESOLVED (WM_USER+1)

HostSolver::HostSolver()
{
	mThread			= NULL;
	mHwndMsg		= NULL; 
	mThreadId		= 0;
	mStartedEvent	= NULL;
	mIsRunning		= false;

	WSADATA	wd;
	int nRet = WSAStartup(MAKEWORD(1, 1), &wd);
	if(nRet != 0) {
		assert(0);
		return ;
	}

	mStartedEvent = CreateEvent(NULL, true, false, NULL);
	assert(mStartedEvent);

	start();
}

HostSolver::~HostSolver()
{
	cancelAll();
	stop();

	if(mStartedEvent) {
		CloseHandle(mStartedEvent);
		mStartedEvent = NULL;
	}
	WSACleanup();
}

bool HostSolver::start()
{
	if(mIsRunning)
		return false;

	assert(mThread == NULL && mHwndMsg == NULL);

	ResetEvent(mStartedEvent);

	mThread = (HANDLE)_beginthreadex(NULL, 0, _solverThread, this, 0, &mThreadId);
	if(!mThread) {
		assert(0);
		return false;
	}

	if(WaitForSingleObject(mStartedEvent, 4000) != WAIT_OBJECT_0) {
		assert(0);
		stop();
		return false;
	}

	if(!mIsRunning) {
		stop();	
		return false;
	}

	return true;
}

void HostSolver::stop()
{
	if(mHwndMsg)
		PostMessage(mHwndMsg, WM_QUIT, 0, 0);

	SetEvent(mStartedEvent);

	if(mThread) {
		if(mThreadId != 0 && mThreadId != GetCurrentThreadId()) {
			WaitForSingleObject(mThread, 5000);
		}
		CloseHandle(mThread);
		mThread = NULL;
	}

	mThreadId = 0;

	if(mHwndMsg) {
		DestroyWindow(mHwndMsg);
		mHwndMsg = NULL;
	}

	mIsRunning = false;
}

void HostSolver::removePending(const char* host)
{
	if(!host)
		return;

	tthread::lock_guard<Mutex> gruad(mMutex);

	if(!mPendingMap.empty()) {
		PendingMap::iterator it = mPendingMap.find(host);
		if(it != mPendingMap.end())
			if(it->second.bufGetHost)
				delete [] (it->second.bufGetHost);

		mPendingMap.erase(host);
	}
		
	if(!mHandle2HostMap.empty()) { 
		Handle2HostMap::iterator it;
		for(it=mHandle2HostMap.begin(); it!=mHandle2HostMap.end(); ++it) {
			if(it->second == host) {
				mHandle2HostMap.erase(it);
				break;
			}
		}
	}
}

void HostSolver::cancel(const char* host)
{
	tthread::lock_guard<Mutex> gruad(mMutex);

	if(mPendingMap.empty())
		return ;

	PendingMap::iterator it = mPendingMap.find(host);
	if(it == mPendingMap.end())
		return ;

	HANDLE handle = it->second.hGetHost;
	if(handle != NULL) {
		WSACancelAsyncRequest(handle);
	}

	notifyListeners(Listener::EVT_CANCELED, host);

	if(it->second.bufGetHost) {
		delete [] it->second.bufGetHost;
	}
	mPendingMap.erase(it);
	mHandle2HostMap.erase(handle);
}

void HostSolver::cancelAll()
{
	tthread::lock_guard<Mutex> gruad(mMutex);

	while(!mPendingMap.empty()) {
		PendingMap::const_iterator it = mPendingMap.begin();
		cancel(it->first.c_str());
	}
	assert(mHandle2HostMap.empty());
}

void HostSolver::notifyListeners(Listener::Event event, const char* host)
{
	assert(host);

	if(mPendingMap.empty())
		return ;

	PendingMap::iterator it = mPendingMap.find(host);
	if(it == mPendingMap.end())
		return;

	std::set<Listener*>& listeners  = it->second.listeners;
	
	std::set<Listener*>::iterator itLn;
	for(itLn=listeners.begin(); itLn!=listeners.end(); ++itLn) {
		if(*itLn) {
			(*itLn)->onSolverEvent(event, host, this);
		}
	}
}

std::string HostSolver::findHostByHandle(HANDLE h) const
{
	if(mHandle2HostMap.empty())
		return "";
	Handle2HostMap::const_iterator it = mHandle2HostMap.find(h);
	if(it == mHandle2HostMap.end())
		return "";
	return it->second;
}


LRESULT CALLBACK HostSolver::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_HOSTRESOLVED:
		{
			if (WSAGETASYNCERROR(lParam)) {
				break;
			}

			HANDLE					handle		= NULL;
			struct hostent*			_hostent	= NULL;
			std::string				host;
			PendingMap::iterator	it;

			HostSolver* thiz = (HostSolver*)GetWindowLong(hWnd, GWLP_USERDATA);
			assert(thiz);

			tthread::lock_guard<Mutex> gruad(thiz->mMutex);
			
			handle = (HANDLE)wParam;
			assert(handle != NULL);

			host = thiz->findHostByHandle(handle);
			if(host == "") {
				assert(0);
				break;
			}
					
			it = thiz->mPendingMap.find(host);
			if(it == thiz->mPendingMap.end()) {
				assert(0);
				break;
			}

			_hostent = (struct hostent*)it->second.bufGetHost;
			assert(_hostent != NULL);

			if(_hostent->h_addrtype == AF_INET)
			{		
				std::vector<unsigned int> ipList;
				for(int i=0; _hostent->h_addr_list[i] != NULL; ++i) {
					unsigned int ip = 0;
					memcpy(&ip, _hostent->h_addr_list[i], _hostent->h_length);
					ipList.push_back(ip);
				}

				thiz->mSolvedMap.insert(std::make_pair(host, ipList));
				thiz->notifyListeners(Listener::EVT_HOSTSOLVED, host.c_str());
				thiz->removePending(host.c_str());
			} else {
				thiz->notifyListeners(Listener::EVT_FAILED, host.c_str());
				thiz->removePending(host.c_str());
			}

		}
		break;
	}

	return 0;
}


HWND HostSolver::createMessageOnlyWindow()
{
	HWND hWnd = CreateWindow(L"STATIC", NULL, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_MESSAGE, NULL, NULL, NULL);
	if(!hWnd) {
		assert(0);
		return NULL;
	}

	SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG)WndProc);
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (long)this);

	return hWnd;
}

void HostSolver::processMessages()
{
	mHwndMsg = createMessageOnlyWindow();
	if(!mHwndMsg) {
		assert(0);	
		mIsRunning = false;
		SetEvent(mStartedEvent);
		return ;
	}

	mIsRunning = true;
	SetEvent(mStartedEvent);

	MSG msg;
	while(GetMessage(&msg, mHwndMsg, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if(msg.message == WM_QUIT)
			break;
	}

	mIsRunning = false;
}

unsigned int HostSolver::_solverThread(void* data)
{
	HostSolver* thiz = (HostSolver*)data;
	assert(thiz);
	thiz->processMessages();
	return 0;
}

bool HostSolver::solve(const char* host)
{
	if(!host || strlen(host) == 0)
		return false;

	assert(mHwndMsg);
	
	tthread::lock_guard<Mutex> gruad(mMutex);

	if(isSolved(host) || isPending(host))
		return true;

	char* buf = new char[MAXGETHOSTSTRUCT];
	PendingData data;
	HANDLE handle = WSAAsyncGetHostByName(mHwndMsg, WM_HOSTRESOLVED, host, buf, MAXGETHOSTSTRUCT);
	if(!handle) {
		delete [] buf;
		return false;
	}
	data.hGetHost = handle;
	data.bufGetHost = buf;

	mPendingMap.insert(std::make_pair(host, data));
	mHandle2HostMap.insert(std::make_pair(handle, host));

	return true;
}

bool HostSolver::solve(const char* host, Listener* ln)
{
	if(!host || strlen(host) == 0)
		return false;

	assert(mHwndMsg);

	tthread::lock_guard<Mutex> gruad(mMutex);

	if(isSolved(host)) {
		if(ln)
			ln->onSolverEvent(Listener::EVT_HOSTSOLVED, host, this);
		return true;
	}

	if(isPending(host)) {
		if(ln) {
			PendingMap::iterator it = mPendingMap.find(host);
			std::set<Listener*>& listeners = it->second.listeners;
			listeners.insert(ln);
		}
		return true;
	}

	char* buf = new char[MAXGETHOSTSTRUCT];
	PendingData data;
	HANDLE handle = WSAAsyncGetHostByName(mHwndMsg, WM_HOSTRESOLVED, host, buf, MAXGETHOSTSTRUCT);
	if(!handle) {
		delete [] buf;
		return false;
	}
	data.hGetHost = handle;
	data.bufGetHost = buf;
	data.listeners.insert(ln);
	mPendingMap.insert(std::make_pair(host, data));
	mHandle2HostMap.insert(std::make_pair(handle, host));

	return true;
}

bool HostSolver::isSolved(const char* host) const
{
	tthread::lock_guard<Mutex> gruad(mMutex);

	if(mSolvedMap.empty())
		return false;
	return mSolvedMap.find(host) != mSolvedMap.end();
}

bool HostSolver::isPending(const char* host) const
{
	if(isSolved(host))
		return false;

	tthread::lock_guard<Mutex> gruad(mMutex);

	if(mPendingMap.empty())
		return false;
	return mPendingMap.find(host) != mPendingMap.end();
}

bool HostSolver::getHostByName(std::vector<unsigned int> &out, const char *host) const
{
	assert(host);

	tthread::lock_guard<Mutex> gruad(mMutex);

	if(mSolvedMap.empty())
		return false;

	SolvedMap::const_iterator it = mSolvedMap.find(host);
	if(it == mSolvedMap.end())
		return false;

	out = it->second;
	if(out.empty())
		return false;

	return true;
}
