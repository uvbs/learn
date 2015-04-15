//======================================================================================
#include "UdpSocket.h"
#include "SocketHandler.h"
#include "selector_epoll.h"

typedef int socklen_t;

//======================================================================================
UdpSocket::UdpSocket(SelectorEPoll* epoll)
:SocketBase(0)
,m_pSelector(epoll)
{

}
//======================================================================================
UdpSocket::~UdpSocket()
{
    m_pSelector->removeSocket((SocketBase*)this);
    CloseSocket();
}

//======================================================================================
bool UdpSocket::Connect(uint32_t uiIP, uint16_t iPort, bool async)
{
    m_iSocket= socket(AF_INET, SOCK_DGRAM, 0);
    if(m_iSocket == -1)
    {
        return false;
    }

    m_ip = uiIP;
    m_iPort = iPort;

    int kOne=1;
    
#ifdef YYMOBILE_IOS
    setsockopt(m_iSocket, SOL_SOCKET, SO_NOSIGPIPE, &kOne, sizeof(kOne));
#elif defined(YYMOBILE_ANDROID)
    setsockopt(m_iSocket, SOL_SOCKET, MSG_NOSIGNAL, &kOne, sizeof(kOne));
#endif        
    setNBlock();
    setRcvBuf(UDP_RECV_BUFF_SIZE);
    setSndBuf(UDP_RECV_BUFF_SIZE);
    m_pSelector->SetEvent(this, 0, SEL_READ);
    return true;
}

//======================================================================================
bool UdpSocket::Listen(uint32_t uiIP, uint16_t& iPort, bool bTryNext)
{
    m_iSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if(m_iSocket == -1 )
    return false ;

    bool bBind = false;
    struct sockaddr_in address;
    memset( &address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(uiIP);
    for(int i = 0; i < 100 ; i++)
    {
        m_iPort = iPort+i;
        address.sin_port = htons(m_iPort);
        if(bind(m_iSocket, (struct sockaddr *) &address, sizeof(address)) != 0)
        {
          //showLog(ERROR_LEVEL, "bind udp socket port: %u failed", m_iPort);
          continue;
        }

        bBind = true;
        break;
    }

	iPort = m_iPort;

    if(bBind == false)
    {
        CloseSocket();
        return false;
    }
    setNBlock();
    setRcvBuf(UDP_RECV_BUFF_SIZE);
    return true;
}
//======================================================================================
int UdpSocket::onReadSocket()
{
    sockaddr_in r_socket;
    socklen_t sock_size = sizeof(r_socket);
    int iRet = 0;

    while(1)
    {
        iRet = recvfrom(m_iSocket, m_sReadBuff, sizeof(m_sReadBuff), 0, (sockaddr*)(&r_socket), &sock_size);
        if(iRet > 0)
        {
          this->m_ip = r_socket.sin_addr.s_addr;
          this->m_iPort = htons(r_socket.sin_port);
          this->m_iReadBuff = iRet;
          iRet = m_pHandler->onDataRecv(m_sReadBuff, m_iReadBuff, this);
        }
        else if( iRet < 0 )
        {
          if(errno == EAGAIN)
            return 0;
          else 
            return -1;
        }
        else
          return iRet;
    }
}
//======================================================================================
int UdpSocket::onWriteSocket()
{
    return 0;
}
//======================================================================================
/*int UdpSocket::SendBin(uint32_t ip, uint16_t port, Marshallable& pPkg, uint32_t uri)
{
    Sender pkg(uri, pPkg);
    pkg.endPack();

    return SendBin(ip, port, pkg.header(), pkg.headerSize() + pkg.bodySize());
}
*/
//======================================================================================
int UdpSocket::SendBin(uint32_t ip, uint16_t port, const char* data, uint32_t len)
{
    if(port == 0)
        return -1;

    struct sockaddr_in address;
    memset( &address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = (ip);
    address.sin_port = htons(port);
    
    int iRet = sendto(m_iSocket, data, len, 0, (struct sockaddr*)&address, sizeof(struct sockaddr));
    if(iRet==-1)
    {
        return -1;
    }
    return iRet;
}
//======================================================================================
void UdpSocket::CloseSocket()
{
#ifdef _WIN32
    closesocket(m_iSocket);
#else
    close(m_iSocket);
#endif

    m_iSocket=-1;
}
//======================================================================================
void UdpSocket::CloseTimeout(time_t tNow)
{
    
}
//======================================================================================
void UdpSocket::setNBlock()
{
#ifdef _WIN32
    unsigned long ul=1;  
    ioctlsocket(m_iSocket, FIONBIO, &ul);
#else
    int fflags = fcntl(m_iSocket, F_GETFL);
    if (-1 == fflags)
    {
        return ;
    }
    fflags |= O_NONBLOCK;
    fcntl(m_iSocket, F_SETFL, fflags);
#endif
}

void UdpSocket::setRcvBuf(uint32_t rcvBufSize)
{
#ifndef _WIN32
    uint32_t optlen  = sizeof(rcvBufSize);
    uint32_t oldSize = 0;
    getsockopt(m_iSocket,SOL_SOCKET,SO_RCVBUF,(void *)&oldSize, (socklen_t*)&optlen); 
    setsockopt(m_iSocket, SOL_SOCKET, SO_RCVBUF, (void *)&rcvBufSize,sizeof(rcvBufSize));   
    getsockopt(m_iSocket,SOL_SOCKET,SO_RCVBUF,(void *)&rcvBufSize, (socklen_t*)&optlen); 
#endif
} 

void UdpSocket::setSndBuf(uint32_t sndBufSize)
{
#ifndef _WIN32
    uint32_t optlen  = sizeof(sndBufSize);
    uint32_t oldSize = 0;
    getsockopt(m_iSocket,SOL_SOCKET,SO_SNDBUF,(void *)&oldSize, (socklen_t*)&optlen); 
    setsockopt(m_iSocket, SOL_SOCKET, SO_SNDBUF, (void *)&sndBufSize,sizeof(sndBufSize));   
    getsockopt(m_iSocket,SOL_SOCKET,SO_SNDBUF,(void *)&sndBufSize, (socklen_t*)&optlen); 
#endif
} 
//======================================================================================

