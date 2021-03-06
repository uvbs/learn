
#ifndef _PACKET_H_INCLUDE__
#define _PACKET_H_INCLUDE__

namespace sox{

#if defined(__i386__)||defined(WIN32) || defined(__x86_64__) || defined(PLATFORM_ANDROID)

	#define XHTONS
	#define XHTONL
	#define XHTONLL

#else /* big end */

inline uint16_t XHTONS(uint16_t i16)
{
	return((i16 << 8) | (i16 >> 8));
}
inline uint32_t XHTONL(uint32_t i32)
{
	return((uint32_t(XHTONS(i32)) << 16) | XHTONS(i32>>16));
}
inline uint64_t XHTONLL(uint64_t i64)
{
	return((uint64_t(XHTONL((uint32_t)i64)) << 32) |XHTONL((uint32_t(i64>>32))));
}

#endif /* __i386__ */

#define XNTOHS XHTONS
#define XNTOHL XHTONL
#define XNTOHLL XHTONLL

}

#endif // _SNOX_PACKET_H_INCLUDE__
