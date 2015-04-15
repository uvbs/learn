//======================================================================================
#include "TcpSocket.h"
#include "selector_epoll.h"
#include "CLogger.h"
#include "SocketHandler.h"
//======================================================================================

#include <cassert>

typedef int socklen_t;

static const char rnd_seed[] = "string to make the random number generator think it has entropy";

#define SOCK_TAG ((std::string("Socket.")+m_dbgname).c_str())

RSA_Helper::RSA_Helper(){
	RAND_seed(rnd_seed, sizeof rnd_seed);

	key =  RSA_generate_key(512, 3, NULL, NULL);

	while(RSA_check_key(key) != 1){
		RSA_free(key);
		key = RSA_generate_key(512, 3, NULL, NULL);
	}
}

RSA_Helper::~RSA_Helper(){
	if(key){
		RSA_free(key);
		CRYPTO_cleanup_all_ex_data();
	}
}

void RSA_Helper::getRSA_PubKey(std::string &pub, std::string &e){
	unsigned char keybuf[1024];

	int size = BN_bn2bin(key->n, keybuf);
	pub = std::string((char *)keybuf, size);

	size = BN_bn2bin(key->e, keybuf);
	//printf("key e size is %u\n", size);
	e = std::string((char *)keybuf, size);
}

void RSA_Helper::rsaDecodeRc4(const std::string &enctext, std::string &rc4key){
	unsigned char rc4buf[100];

	int num = RSA_private_decrypt(enctext.length(), (const unsigned char *)enctext.data(), rc4buf, key,	RSA_PKCS1_PADDING);
	if (num == -1){
		assert(false);
		rc4key = "";
		return;
	}
	rc4buf[num] = 0;
	rc4key = std::string((const char *)rc4buf, num);
}
#define ENCODE_BUFFER 1024*1024
//======================================================================================

RC4Alloc::RC4Alloc() {
	writeBuffer = new unsigned char[ENCODE_BUFFER];
	curSize = ENCODE_BUFFER;
}
//======================================================================================

RC4Alloc::~RC4Alloc() {
	delete[] writeBuffer;
}
//======================================================================================

void RC4Alloc::realloc(size_t sz) {
	delete[] writeBuffer;
	writeBuffer = new unsigned char[sz];
	curSize = sz;
	log(Info, "RC4Alloc::realloc %u", sz);
}
//======================================================================================

RC4Alloc RC4Filter::writeBuffer;

//======================================================================================

RC4Filter::RC4Filter() {
	encrypted = false;
}
//======================================================================================

RC4Filter::~RC4Filter() {

}

//======================================================================================

void RC4Filter::filterRead(char *data, size_t sz) {
	if (encrypted) {
		RC4(&rc4, sz, (unsigned char *) data, (unsigned char *) data);
	}
}
//======================================================================================

char* RC4Filter::filterWrite(char *data, size_t sz) {
	if (encrypted) {
		if (sz > writeBuffer.getCurSize()) {
			//log(Info, "RC4Filter::filterWrite size:%u", sz);
			writeBuffer.realloc(sz);
		}
		RC4(&rc4, sz, (unsigned char *) data, writeBuffer.getWriteBuffer());
		return (char *) writeBuffer.getWriteBuffer();
	} else {
		//log(Info, "RC4Filter::filterWrite not encrypted size:%u", sz);
		return data;
	}

}
//======================================================================================

bool RC4Filter::isEncrypto() const {
	return encrypted;
}
//======================================================================================

void RC4Filter::setRC4Key(const unsigned char *data, size_t len) {
	RC4_set_key(&rc4, len, data);
	encrypted = true;
}

//======================================================================================
TcpSocket::TcpSocket(SelectorEPoll* epoll, const char* dbgName) :
SocketBase(1), m_pSelector(epoll), m_bListenSocket(false), m_bEnanbe(true),
m_bConnected(false), m_iLastRecvTime(0), m_dbgname(dbgName ? dbgName : "Unknown") {
	m_SockData.uid = -1;
	m_SockData.sid = -1;
	m_SockData.role = -1;
}
//======================================================================================
TcpSocket::~TcpSocket() {
	Log.i(SOCK_TAG, "TcpSocket::~TcpSocket for socket:%u addr:%s", m_iSocket, ip2str(m_ip));
	m_pSelector->removeSocket((SocketBase*) this);
	CloseSocket();
	m_SockData.uid = -1;
	m_SockData.sid = -1;
	m_SockData.role = -1;
	m_bConnected = false;
	m_bListenSocket = false;
	m_pSelector = NULL;
}

//======================================================================================
void TcpSocket::CloseTimeout(time_t tNow) {
	if ((m_bListenSocket) || (m_iTimeout == 0))
		return;

	if (m_bEnanbe == false || SelectorEPoll::getInstance().m_iHaoMiao > (m_iLastRecvTime + m_iTimeout)) {
		Log.i(SOCK_TAG, "TcpSocket close timeout connection socket:%u addr m_bEnable %s m_iLastRecvTime %u m_iHaoMiao %u",
			m_iSocket, m_bEnanbe?"enabled":"unenabled", m_iLastRecvTime, SelectorEPoll::getInstance().m_iHaoMiao);
		m_pHandler->onClose(this);
	}
}
//======================================================================================
bool TcpSocket::setListenSockt(bool isListen) {
	m_bListenSocket = isListen;
	return true;
}

//======================================================================================
int TcpSocket::onReadSocket() {
	m_iLastRecvTime = SelectorEPoll::getInstance().m_iHaoMiao;
	if (m_pHandler == NULL){
		Log.d(SOCK_TAG, "TcpSocket::onReadSocket m_pHandler == NULL.");
		return -1;
	}
	if (m_bListenSocket){
		Log.d(SOCK_TAG, "TcpSocket::onReadSocket is Listen Socket.");
		m_pHandler->SocketRead(this);
		return 0;
	}

	if(m_bEnanbe == false){
		return 0;
	}

	int ret = 0;
	/* begin change by duzf */
#if 0
	try {
#endif
		/* end change by duzf */	
		ret = m_input.pump(*this);
		if( ret > 0)
		{
			ret = m_pHandler->onDataRecv(m_input.data(), m_input.size(), this);
			if (ret != -1)
			{
				m_input.erase(0, ret);
			}
			else
			{
				Log.e(SOCK_TAG,"TcpSocket::onReadSocket close the socket initiative socket local:%u addr:%s.",m_iSocket, ip2str(m_ip));
				m_pHandler->onClose(this);
			}
		}
		else
		{
			Log.w(SOCK_TAG, "TcpSocket::onReadSocket close the socket reset by peer socket local:%u %s:%u.",m_iSocket, ip2str(m_ip), m_iPort);
			m_pHandler->onClose(this);//peer had closed
		}
		/* begin change by duzf */
#if 0
	} catch (sox::UnpackError &se) {
		log(Error, "excpetion, %s, %s", (std::string("Inner Conn read error:")+ se.what()).data(), ip2str(m_ip));
		m_pHandler->onClose(this);

	} catch (std::exception &ex) {
		log(Error, "excpetion, %s,%s", (std::string("Inner Conn read error:")+ ex.what()).data(), ip2str(m_ip));
		m_pHandler->onClose(this);
	}
#endif
	/* end change by duzf */
	return ret;
}
//======================================================================================
int TcpSocket::onWriteSocket() {
	if(m_bEnanbe == false){
		Log.e(SOCK_TAG, "TcpSocket::onWriteSocket m_bEnanbe == false");
		m_pSelector->SetEvent(this, SEL_WRITE, 0);
		return 0;
	}
	if (m_bConnected == false)
	{
		m_bConnected = true;
		m_pHandler->onConnected(this);
		return 0;
	}
	int res = m_output.flush(m_iSocket);
	if(res < 0){
		Log.e(SOCK_TAG,"TcpSocket::onWriteSocket close the socket reset by peer when send:%u ip:%s",m_iSocket, ip2str(m_ip));
		m_bEnanbe = false;
		m_pSelector->SetEvent(this, SEL_WRITE, 0);
		m_pHandler->onClose(this);
	}else{
		if (m_output.empty()){
			//log(Debug, "onWrite SetEvent remove");
			m_pSelector->SetEvent(this, SEL_WRITE, 0);
		}
	}
	return 0;
}

void TcpSocket::onError() {
	if (m_pHandler){
		Log.i(SOCK_TAG, "TcpSocket::onError  close socket");
		m_pHandler->onClose(this);
	}
}
//======================================================================================
int TcpSocket::SendBin(uint32_t ip, uint16_t port, const char* data, uint32_t len) {
	if (m_output.max_blocks < m_output.block()) {
		Log.e(SOCK_TAG, "tcp socket send buffer error max block:%u current:%u, %s",
			m_output.max_blocks, m_output.block(), ip2str(m_ip));
		return 0;
	}

	if (m_bEnanbe) {
		if (m_bConnected) {
			//m_output.write(*this, (char*) data, len);
			m_output.pushData(data, len);
			int res = m_output.flush(m_iSocket);
			if(res < 0){
				Log.w(SOCK_TAG, "TcpSocket::SendBin close the socket reset by peer socketL:%u addr:%s",m_iSocket, ip2str(m_ip));
				m_pHandler->onClose(this);
				m_bEnanbe = false;
				return 0;
			}
			if (!m_output.empty()){
				m_pSelector->SetEvent(this, 0, SEL_WRITE);
			}
		}
	} else {
		//log(Info, "socket not enable: %s", sox::addr_ntoa(peerIp).data());
		Log.i(SOCK_TAG, "socket not enable");
	}
	return 0;
}
//======================================================================================
void TcpSocket::CloseSocket() {
	if (m_iSocket == -1)
		return;

	Log.d(SOCK_TAG, "TcpSocket::CloseSocket iSocket: %u, %s", m_iSocket, ip2str(m_ip));

	closesocket(m_iSocket);
	m_iSocket = -1;
	m_bConnected = false; // gs 
}
//======================================================================================
bool TcpSocket::Connect(uint32_t uiIP, uint16_t iPort, bool async)
{
	Log.i(SOCK_TAG, "TcpSocket::Connect uiIp:%u. iPort:%u, async:%s", uiIP, iPort, async ? "async" : "sync");
	WSADATA	wd;
	int result = WSAStartup(MAKEWORD(1, 1), &wd);
	if(result != 0){
		Log.e(SOCK_TAG, "WSAStartup failed, error code %u", result);
		return false;
	} 
	m_iSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_iSocket == -1){
		int errorCode = WSAGetLastError();
		Log.e(SOCK_TAG, "TcpSocket::Connect socket fail, error code %u.", errorCode);
		return false;
	}

	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = uiIP;
	address.sin_port = htons(iPort);

	m_ip = uiIP;
	m_iPort = htons(iPort);

	if (async)
	{
		Log.i(SOCK_TAG, "Set non block.");
		m_bConnected = false;
		SetNBlock();
		m_iLastRecvTime = SelectorEPoll::getInstance().m_iHaoMiao;
		m_pSelector->SetEvent(this, 0, SEL_RW);
	}

	if (connect(m_iSocket, (struct sockaddr*) &address, sizeof(address)) == -1) {
		int errorCode = WSAGetLastError();
		if (errorCode == WSAEWOULDBLOCK){
			Log.d(SOCK_TAG, "TcpSocket::Connect in progress. ip=%s", ip2str(uiIP));
			return true;
		}
		Log.w(SOCK_TAG, " TcpSocket::Connect %s:%u fail. error code: %u", ip2str(uiIP), iPort, errorCode);
		CloseSocket();
		return false;
	}

	m_iLastRecvTime = SelectorEPoll::getInstance().m_iHaoMiao;
	m_bConnected = true;
	SetNBlock();
	m_pSelector->SetEvent(this, 0, SEL_RW);
	return true;
}
//======================================================================================
bool TcpSocket::Listen(uint32_t uiIP, uint16_t &iPort, bool bTryNext) {
	m_iSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_iSocket == -1) {
		Log.e(SOCK_TAG, "TcpSocket::Listen get socket fail");
		return false;
	}

#if 0
	int op = 1;
	if (-1 == setsockopt(m_iSocket, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op))) {
		log(Error, "TcpSocket::Listen set socket fail");
		CloseSocket();
		return false;
	}
#endif

	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = uiIP;
	for (int i = 0; i < 100; i++) {
		m_iPort = iPort;
		address.sin_port = htons(m_iPort);
		if (bind(m_iSocket, (struct sockaddr *) &address, sizeof(address)) < 0) {
			//log(Debug, "TcpSocket bind tcp socket port: %u failed", m_iPort);
			iPort++;
			continue;
		}

		//bind success
		if (listen(m_iSocket, SOMAXCONN) == -1) {
			Log.e(SOCK_TAG, "TcpSocket listen tcp socket port %u failed", m_iPort);
			CloseSocket();
			return false;
		}

		SetNBlock();
		//log(Info, "bind tcp socket:%u port:%u successed", m_iSocket, m_iPort);
		return true;
	}
	CloseSocket();
	Log.e(SOCK_TAG, "bind tcp socket port: %u-%u fails", m_iPort-100, m_iPort);
	return false;
}
//======================================================================================
TcpSocket* TcpSocket::Accept() {
	struct sockaddr_in sa;
	socklen_t len = sizeof(sa);
	Log.i(SOCK_TAG, "TcpSocket::Accept m_iSocket=%u", m_iSocket);
	int ret = accept(m_iSocket, (struct sockaddr*) &sa, &len);
	if (-1 == ret){
		Log.e(SOCK_TAG, "TcpSocket::Accept -1!");
		return NULL;
	}
	if (0 == ret) {
		Log.e(SOCK_TAG, "TcpSocket::Accept 0!");
		return NULL;
	}

	TcpSocket *pNew = new TcpSocket(m_pSelector);
	pNew->m_iSocket = ret;
	//pNew->m_ip = ntohl(sa.sin_addr.s_addr);
	//pNew->m_iPort = ntohs(sa.sin_port);
	pNew->m_ip = sa.sin_addr.s_addr;
	pNew->m_iPort = sa.sin_port;
	pNew->m_bConnected = true;
	pNew->m_iLastRecvTime = SelectorEPoll::getInstance().m_iHaoMiao;
	pNew->setTimeout(m_iTimeout);
	pNew->SetNBlock();
	Log.i(SOCK_TAG, "TcpSocket accept %s:%u", ip2str(pNew->m_ip), pNew->m_iPort);
	return pNew;
}
//======================================================================================
void TcpSocket::SetNBlock() {
	// If iMode!=0, non-blocking mode is enabled.
	u_long iMode=1;
	int result = ioctlsocket(m_iSocket,FIONBIO,&iMode);
	if(result != 0){
		Log.e(SOCK_TAG, "set NBlock error, socket id:%u, ip=%s", m_iSocket, ip2str(m_ip));
		return;
	}
	/*
	int fflags = fcntl(m_iSocket, F_GETFL);
	if (-1 == fflags) {
	log(Error, "set NBlock error, socket id:%u, ip=%s", m_iSocket, ip2str(m_ip));
	return;
	}
	fflags |= O_NONBLOCK;
	fcntl(m_iSocket, F_SETFL, fflags);
	*/
}
//======================================================================================
void TcpSocket::SetRC4Key(const unsigned char *data, size_t len) {
	m_output.setRC4Key(data, len);
	m_input.setRC4Key(data, len);
}

uint16_t TcpSocket::getLocalPort()
{
	struct sockaddr_in sa;
	socklen_t len = sizeof(sa);
	if ( getsockname(m_iSocket, (struct sockaddr*) &sa, &len) != 0 )
	{
		return 0;
	}

	return ntohs(sa.sin_port);
}
//======================================================================================
//======================================================================================
//======================================================================================
//end
