//======================================================================================
#ifndef _TCPSOCKET_HEADER_
#define _TCPSOCKET_HEADER_
//======================================================================================
#include <openssl/rc4.h>
#include <openssl/sha.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
//======================================================================================
#include <string>
#include "packet.h"
#include "SocketBase.h"
#include "blockbuffer.h"
#include "sockbuffer.h"
#include "sendBuffer.h"

//#define DEF_SESSIONKEY_LENGTH 16
//======================================================================================
class SelectorEPoll;
//======================================================================================

class RSA_Helper{
public:
	RSA_Helper();
	~RSA_Helper();

	void getRSA_PubKey(std::string &pub, std::string &e);

	void rsaDecodeRc4(const std::string &enctext, std::string &rc4key);
	
	RSA* getRsaKey() const { return (RSA*)key; }

private:
	RSA *key;
};


struct RSAHelper_Aware{
	//void set_rsahelper(RSA_Helper *rh) {rsahelper = rh;}

protected:
	RSA_Helper rsahelper;
};
//======================================================================================
struct RC4Alloc {
	RC4Alloc();
	~RC4Alloc();

	unsigned char *writeBuffer;
	size_t curSize;

	unsigned char *getWriteBuffer() {
		return writeBuffer;
	}
	size_t getCurSize() {
		return curSize;
	}
	void realloc(size_t sz);
};
//======================================================================================
class RC4Filter {
	bool encrypted;
	RC4_KEY rc4;
	static RC4Alloc writeBuffer;
public:
	RC4Filter();
	~RC4Filter();
public:
	void filterRead(char *, size_t);
	char* filterWrite(char *, size_t);
	bool isEncrypto() const;
	void setRC4Key(const unsigned char *, size_t);
};
//======================================================================================
typedef BlockBuffer<def_block_alloc_8k, 9> Buffer8x9k; // 72K WOW!
typedef BlockBuffer<def_block_alloc_8k, 128 * 8> Buffer8x128k; // 1M WOW!

typedef SockBuffer<Buffer8x128k, RC4Filter> InputBuffer;
typedef SockBuffer<Buffer8x128k, RC4Filter> OutputBuffer;
//======================================================================================
struct UserConnData {
	uint32_t uid;
	uint32_t sid;
	uint32_t role;
};
//======================================================================================
class TcpSocket: public SocketBase {
public:
	TcpSocket(SelectorEPoll* epoll, const char* dbgName = NULL);
	virtual ~TcpSocket();

public:
	bool Listen(uint32_t uiIP, uint16_t &iPort, bool bTryNext);
	virtual bool Connect(uint32_t uiIP, uint16_t iPort, bool async = false);
	void CloseTimeout(time_t tNow);
	virtual void CloseSocket();
	void* getParamData() {
		return &m_SockData;
	}
	//void  setParamData(void* param) {  m_pParam = param; }
	virtual void setTimeout(uint32_t interval) {
		m_iTimeout = interval;
	}
	uint32_t getTrustedUid() const {
		return m_SockData.uid;
	}
	uint32_t getSid() const {
		return m_SockData.sid;
	}

	TcpSocket *Accept();

	int onReadSocket();
	int onWriteSocket();
	int SendBin(uint32_t ip, uint16_t port, const char* data, uint32_t len);

	bool isConnected() const {
		return m_bConnected;
	}
	
	bool isOutputBufferEmpty() const {
		return m_output.empty() ? true : false;
	}

	bool setListenSockt(bool isListen);
	void SetRC4Key(const unsigned char *data, size_t len);
	//void setRC4Key(const unsigned char *data, size_t len);
	void setEnable(bool be) {
		m_bEnanbe = be;
	}
	void onError();

	uint16_t getLocalPort();


protected:
	void SetNBlock();

public:
	InputBuffer m_input;
	//OutputBuffer m_output;
	SendBuffer m_output;

	SelectorEPoll* m_pSelector;
	UserConnData m_SockData;
	bool m_bListenSocket;
	bool m_bEnanbe;
	bool m_bConnected;
	uint32_t m_iLastRecvTime;
	const std::string m_dbgname;
};
//======================================================================================

//======================================================================================
#endif

