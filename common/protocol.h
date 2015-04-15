//======================================================================================
#ifndef _PROTOCOL_HEADER_
#define _PROTOCOL_HEADER_
//======================================================================================
#include "common.h"
#include "packet.h"

//======================================================================================

//using namespace sox;

namespace protocol {
	namespace session {
		struct IpInfo{
			uint32_t ip;
			std::vector<uint16_t> tcpPorts;
			std::vector<uint16_t> udpPorts;
		};
	}
}



#endif
//======================================================================================

