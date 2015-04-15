// SocketBase.h: interface for the SocketBase class.
//
//======================================================================================
#ifndef _SOCKET_BASE_
#define _SOCKET_BASE_
//======================================================================================
#include "common.h"
#include "packet.h"
#include "protocol.h"
#include "assert.h"
//======================================================================================

class SocketHandler;

class SocketBase {
public:
	enum SocketType
	{
		TYPE_UDP = 0,
		TYPE_TCP,
	};

	SocketBase(uint8_t type = -1){
		m_ip = -1;
		m_iPort = -1;
		m_iSocketType = -1;
		m_iSocket = -1;
		m_pParam = NULL;
		m_pHandler = NULL;
		m_iTimeout = 0;
		m_iSocketType = type;
	}

	virtual ~SocketBase() {
		//m_ip = -1;
		//m_iPort = -1;
		//m_iSocketType = -1;
		//m_iSocket = -1;
		m_pParam = NULL;
		m_pHandler = NULL;
	}

	virtual bool Listen(uint32_t uiIP, uint16_t &iPort, bool bTryNext) = 0;
	virtual bool Connect(uint32_t uiIP, uint16_t iPort, bool async) = 0;
	virtual void CloseTimeout(time_t tNow) = 0;
	virtual int onReadSocket() = 0;
	virtual int onWriteSocket() = 0;
	virtual void onError() = 0;
	virtual void CloseSocket() = 0;
	virtual void* getParamData() = 0;
	virtual int SendBin(uint32_t ip, uint16_t port, const char* p, uint32_t len) = 0;
	//virtual void  setParamData(void* param) = 0;
	virtual void setTimeout(uint32_t interval) = 0;
	virtual int fd(){return m_iSocket;}

protected:
	uint32_t m_iTimeout;

public:
	uint32_t m_ip;
	uint16_t m_iPort;
	uint8_t m_iSocketType; // 0:udp,1:tcp

	int m_iSocket;
	void *m_pParam;
	SocketHandler *m_pHandler;

	bool m_bEnanbe;

	int m_selEvent;
};
//======================================================================================
class WrapForwardBuffer {
public:
	WrapForwardBuffer(const char * data, size_t size) :
	  m_data(data), m_size(size), m_pos(0) {
	  }
public:

	const char * data() {
		return m_data + m_pos;
	}
	size_t size() {
		return m_size - m_pos;
	}

	void erase(size_t n) {
		m_pos += n;
		assert(m_pos <= m_size);
	}
	bool empty() const {
		return m_size == m_pos;
	}
	size_t offset() const {
		return m_pos;
	}

private:
	const char * m_data;
	size_t m_size;
	size_t m_pos;
};
//======================================================================================
#endif

