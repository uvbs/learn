/*
* SocketHandler.cpp
*
*  Created on: 2010-2-17
*      Author: Administrator
*/
#include "SocketHandler.h"
#include "TcpSocket.h"

TcpSocket* SocketHandler::createTcpListener(uint32_t uiIP, uint16_t& iPort,
					SocketHandler *handler, uint32_t timeout, const char* dbgname)
{
	TcpSocket* listenRes = new TcpSocket(&SelectorEPoll::getInstance(), dbgname);
	if(listenRes->Listen(uiIP, iPort, true) == false)
	{
		log(Error, "ListenManager tcp port %u listen failed", iPort);
		return NULL;
	}
	listenRes->m_pHandler = handler;
	listenRes->setListenSockt(true);
	listenRes->setTimeout(timeout);
	//log(Debug, "Calling g_epoll.SetEvent");
	SelectorEPoll::getInstance().SetEvent(listenRes, 0, SEL_READ);
	return listenRes;
}
