//======================================================================================
#ifndef _UDP_SOCKET_HEADER_
#define _UDP_SOCKET_HEADER_
//======================================================================================
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

//======================================================================================
#define RECV_BUFF_SIZE 12*1024
#define UDP_RECV_BUFF_SIZE 512*1024
//======================================================================================
class SelectorEPoll;

class UdpSocket : public SocketBase
{
public:
  UdpSocket(SelectorEPoll* epoll);
  virtual ~UdpSocket();

public:
  bool Connect(uint32_t uiIP, uint16_t iPort, bool async);
  bool Listen(uint32_t uiIP,uint16_t& iPort, bool bTryNext);
  void CloseTimeout(time_t tNow);
  int onReadSocket();
  int onWriteSocket();
  void CloseSocket();
  void onError() {};
  void* getParamData() { return NULL; }
  void setTimeout(uint32_t interval) {}

public:
  //int SendBin(uint32_t ip, uint16_t port, Marshallable& pPkg, uint32_t uri);
  int SendBin(uint32_t ip, uint16_t port, const char* data, uint32_t len);

  void setNBlock();
  void setRcvBuf(uint32_t rcvBufSize);
  void setSndBuf(uint32_t sndBufSize);

public:
    SelectorEPoll* m_pSelector;
    char m_sReadBuff[RECV_BUFF_SIZE];
    int m_iReadBuff;
};

//======================================================================================
//======================================================================================
#endif

