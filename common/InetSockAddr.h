#ifndef __InetSockAddr_H
#define __InetSockAddr_H

struct InetSockAddr
{
	uint32_t ip;
	uint16_t port;

	bool operator==(const InetSockAddr & addr) {
		return ((ip == addr.ip) && (port == addr.port));
	}

	bool operator>(const InetSockAddr & addr) {
		return ((ip > addr.ip) || ((ip == addr.ip) && (port > addr.port)));
	}
};

#endif //__InetSockAddr_H