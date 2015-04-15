#include <winsock2.h>
#include <mswsock.h>
#include <assert.h>
#include <process.h>
#include <stdio.h>
#include <vector>
#include "NetworkStatus.h"
#include "CLogger.h"

#define TAG		"NetworkStatus"

#define ENABLE_DEBUG	1

#if ENABLE_DEBUG
#define dprintf(fmt, ...)	Log.i(TAG, fmt, ##__VA_ARGS__)
#else
#define dprintf(...)
#endif

NetworkStatus::NetworkStatus()
{
	mThread					= NULL;
	mThreadId				= 0;
	mIsRunning				= false;
	mStop					= false;
	mCurrentStatus			= INTERNET_UNKNOWN;
	mStartedEvent			= NULL;
	mWsaStatusChanngeEvent	= WSA_INVALID_EVENT;

	WSADATA	wd;
	int nRet = WSAStartup(MAKEWORD(1, 1), &wd);
	if(nRet != 0) {
		assert(0);
		Log.e(TAG, "%s WSAStartup failed, err=%d", __FUNCTION__, WSAGetLastError());
		return ;
	}

	mStartedEvent = CreateEvent(NULL, true, false, NULL);
	assert(mStartedEvent);

	mWsaStatusChanngeEvent = WSACreateEvent();
	assert(mWsaStatusChanngeEvent != WSA_INVALID_EVENT);

	start();
}

NetworkStatus::~NetworkStatus()
{
	stop();

	removeAllListeners();

	if(mStartedEvent) {
		CloseHandle(mStartedEvent);
		mStartedEvent = NULL;
	}

	if(mWsaStatusChanngeEvent != WSA_INVALID_EVENT) {
		WSACloseEvent(mWsaStatusChanngeEvent);
		mWsaStatusChanngeEvent = WSA_INVALID_EVENT;
	}

	WSACleanup();
}

bool NetworkStatus::start()
{
	if(mIsRunning)
		return false;

	assert(mThread == NULL && mStartedEvent && mWsaStatusChanngeEvent != WSA_INVALID_EVENT);

	ResetEvent(mStartedEvent);
	WSAResetEvent(mWsaStatusChanngeEvent);

	mStop = false;

	mThread = (HANDLE)_beginthreadex(NULL, 0, networkStatusThread, this, 0, &mThreadId);
	if(!mThread) {
		assert(0);
		return false;
	}

	if(WaitForSingleObject(mStartedEvent, 5000) != WAIT_OBJECT_0) {
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


void NetworkStatus::stop()
{
	mStop = true;

	SetEvent(mStartedEvent);
	WSASetEvent(mWsaStatusChanngeEvent);

	if(mThread) {
		if(mThreadId != 0 && mThreadId != GetCurrentThreadId()) {
			if(WaitForSingleObject(mThread, 5000) != WAIT_OBJECT_0) {
				Log.w(TAG, "stop thread failed");
			}
		}
		CloseHandle(mThread);
		mThread = NULL;
	}

	mThreadId = 0;

	mIsRunning = false;
}

int NetworkStatus::getCurrentStatus() const
{
	return mCurrentStatus;
}

void NetworkStatus::monitorNetorkStatus()
{
    while(!mStop)
    {
		std::vector<char>	buff;
		DWORD				bufferSize;
		WSAQUERYSET*		qs=NULL;
		GUID				NLANameSpaceGUID = NLA_SERVICE_CLASS_GUID;
		HANDLE				hNLA;

		// Initalize the WSAQUERYSET structure for the query
		//
		buff.resize(64*1024);
		qs = (WSAQUERYSET *) &(buff[0]);
		memset(qs, 0, sizeof(*qs));
		// Required values
		qs->dwSize					= sizeof(WSAQUERYSET);
		qs->dwNameSpace				= NS_NLA;
		qs->lpServiceClassId		= &NLANameSpaceGUID;
		// Optional values
		qs->dwNumberOfProtocols		= 0;
		qs->lpszServiceInstanceName	= NULL;
		qs->lpVersion				= NULL;
		qs->lpNSProviderId			= NULL;
		qs->lpszContext				= NULL;
		qs->lpafpProtocols			= NULL;
		qs->lpszQueryString			= NULL;
		qs->lpBlob					= NULL;
	    
		if (WSALookupServiceBegin(qs, LUP_RETURN_ALL|LUP_DEEP, &hNLA) == SOCKET_ERROR)
		{
			Log.e(TAG, "WSALookupServiceBegin failed with error %d\n", 
				   WSAGetLastError());
			return;
		}

        DWORD				bytesReturned;
        WSACOMPLETION		WSAComplete;
        WSAOVERLAPPED		WSAOverlap;
		bool				internetConnected  = false;
		bool				foundConnectiveType = false;

        dprintf("Querying for Networks...\n");
        while (!mStop)
        {
            memset(qs, 0, sizeof(*qs));
            bufferSize = buff.size();
            if (WSALookupServiceNext(hNLA, LUP_RETURN_ALL, &bufferSize, qs) == SOCKET_ERROR)
            {
                int Err = WSAGetLastError();
                if (Err == WSA_E_NO_MORE)
                {
                    // There is no more data. Stop asking.
                    //
                    break;
                }
                dprintf("WSALookupServiceNext failed with error %d\n",
                       WSAGetLastError());
                WSALookupServiceEnd(hNLA);
                return;
            }
            dprintf("\nNetwork Name: %s\n", qs->lpszServiceInstanceName);
            dprintf("Network Friendly Name: %s\n", qs->lpszComment);

            if (qs->lpBlob != NULL)
            {
                //
                // Cycle through BLOB data list
                //
                DWORD			Offset = 0;
                PNLA_BLOB		pNLA;
                do
                {
                    pNLA = (PNLA_BLOB) &(qs->lpBlob->pBlobData[Offset]);
                    switch (pNLA->header.type)
                    {
                        case NLA_RAW_DATA:
                            dprintf("\tNLA Data Type: NLA_RAW_DATA\n");
                            break;
                        case NLA_INTERFACE:
                            dprintf("\tNLA Data Type: NLA_INTERFACE\n");
                            dprintf("\t\tType: %dn", pNLA->data.interfaceData.dwType);
                            dprintf("\t\tSpeed: %dn", pNLA->data.interfaceData.dwSpeed);
                            dprintf("\t\tAdapter Name: %s\n", pNLA->data.interfaceData.adapterName);
                            break;
                        case NLA_802_1X_LOCATION:
                            dprintf("\tNLA Data Type: NLA_802_1X_LOCATION\n");
                            dprintf("\t\tInformation: %sn", pNLA->data.locationData.information);
                            break;
                        case NLA_CONNECTIVITY:
                            dprintf("\tNLA Data Type: NLA_CONNECTIVITY\n");
	                        switch(pNLA->data.connectivity.type)
                            {
                                 case NLA_NETWORK_AD_HOC:
                                     dprintf("\t\tNetwork Type: AD HOC\n");
									 foundConnectiveType = true;
                                     break;
                                 case NLA_NETWORK_MANAGED:
                                     dprintf("\t\tNetwork Type: Managed\n");
									 foundConnectiveType = true;
                                     break;
                                 case NLA_NETWORK_UNMANAGED:
                                     dprintf("\t\tNetwork Type: Unmanaged\n");
									 foundConnectiveType = true;
                                     break;
                                 case NLA_NETWORK_UNKNOWN:
                                     dprintf("\t\tNetwork Type: Unknown\n");
									 break;
                            }
                            switch(pNLA->data.connectivity.internet)
                            {
                                 case NLA_INTERNET_NO:
                                     dprintf("\t\tInternet connectivity: No\n");
                                     break;
                                 case NLA_INTERNET_YES:
                                     dprintf("\t\tInternet connectivity: Yes\n");
									 internetConnected = true;
                                     break;
                                 case NLA_INTERNET_UNKNOWN:
                                     dprintf("\t\tInternet connectivity: Unknown\n");
                                     break;
                            }
                            break;
                        case NLA_ICS:
                            dprintf("\tNLA Data Type: NLA_ICS\n");
                            dprintf("\t\tSpeed: %d\n", pNLA->data.ICS.remote.speed);
                            dprintf("\t\tType: %d\n", pNLA->data.ICS.remote.type);
                            dprintf("\t\tState: %d\n", pNLA->data.ICS.remote.state);
                            dprintf("\t\tMachine Name: %S\n", pNLA->data.ICS.remote.machineName);
                            dprintf("\t\tShared Adapter Name: %S\n", pNLA->data.ICS.remote.sharedAdapterName);
                            break;
                        default:
                            Log.e(TAG, "\tNLA Data Type: Unknown to this program");
                            break;
                    }
                    Offset += pNLA->header.nextOffset;
                } while (pNLA->header.nextOffset != 0);
            }
        }

		if(mStop) {
			Log.i(TAG, "already stopped");
			WSALookupServiceEnd(hNLA);
			return ;
		}

		int oldStatus = mCurrentStatus;
		if(!foundConnectiveType) {
			mCurrentStatus = INTERNET_UNKNOWN;
		} else {
			mCurrentStatus = internetConnected ? INTERNET_CONNECTED : INTERNET_DISCONNECTED;
		}
		notifyListeners(mCurrentStatus, oldStatus);
		Log.i(TAG, "internet connectivity %s", internetConnected ? "Yes" : "No");

        dprintf("\nFinished query, Now wait for change notification...\n");
        WSAOverlap.hEvent = mWsaStatusChanngeEvent;
        WSAComplete.Type = NSP_NOTIFY_EVENT;
        WSAComplete.Parameters.Event.lpOverlapped = &WSAOverlap;
        if (WSANSPIoctl(hNLA, SIO_NSP_NOTIFY_CHANGE, NULL, 0, NULL, 0,
            &bytesReturned, &WSAComplete) == SOCKET_ERROR)
        {
            int Ret = WSAGetLastError();
            if (Ret != WSA_IO_PENDING)
            {
                Log.e(TAG, "WSANSPIoctrl failed with error %d\n", Ret);
                return;
            }
        }
        if (WSAWaitForMultipleEvents(1, &mWsaStatusChanngeEvent, TRUE, WSA_INFINITE, FALSE) == WSA_WAIT_FAILED)
        {
            Log.e(TAG, "WSAWaitForMultipleEvents failed with error %d\n", WSAGetLastError());
            return;
        }
        WSAResetEvent(mWsaStatusChanngeEvent);

		WSALookupServiceEnd(hNLA);
    }
}

bool NetworkStatus::addListener(IStatusChangeListener* ln)
{
	if(!ln)
		return false;

	tthread::lock_guard<Mutex> lockGrard(mListenerMutex);

	mListeners.insert(ln);

	return true;
}

void NetworkStatus::removeListener(IStatusChangeListener* ln)
{
	tthread::lock_guard<Mutex> lockGrard(mListenerMutex);

	if(!mListeners.empty())
		mListeners.erase(ln);
}

void NetworkStatus::removeAllListeners()
{
	tthread::lock_guard<Mutex> lockGrard(mListenerMutex);

	if(!mListeners.empty())
		mListeners.clear();
}

void NetworkStatus::notifyListeners(int curStatus, int oldStatus)
{
	tthread::lock_guard<Mutex> lockGrard(mListenerMutex);

	for(std::set<IStatusChangeListener*>::iterator it = mListeners.begin(); it != mListeners.end(); ++it) {
		if(*it) {
			(*it)->onStatusChange(curStatus, oldStatus);
		}
	}
}

unsigned int NetworkStatus::networkStatusThread(void* data)
{
	assert(data != NULL);
	NetworkStatus* thiz = (NetworkStatus*)data;
	
	thiz->mIsRunning = true;

	SetEvent(thiz->mStartedEvent);

	thiz->monitorNetorkStatus();

	thiz->mIsRunning = false;

	thiz->mCurrentStatus = INTERNET_UNKNOWN;
	
	return 0;
}
