#ifndef _SOCKET_HANDLER_HEADER_
#define _SOCKET_HANDLER_HEADER_

#include "TcpSocket.h"
#include "selector_epoll.h"

#define CONNECTION_TIMEOUT 60*1000

class SocketBase;

class SocketHandler
{
public:
	virtual ~SocketHandler(){};
	virtual int SocketRead(SocketBase*) = 0;
	virtual int SocketWrite(SocketBase*) = 0; //only for list socket;
	virtual int onDataRecv(const char*, int, SocketBase*) = 0;
	virtual int onClose(SocketBase*) = 0;
	virtual void onConnected(SocketBase*) = 0;
	//virtual int onConnected() = 0;
	static TcpSocket* createTcpListener(uint32_t uiIP,uint16_t& iPort,
		SocketHandler *handler, uint32_t timeout, const char* dbgname);

};


#endif



