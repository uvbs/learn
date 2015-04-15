#ifndef UTILITY_H
#define UTILITY_H

#include "common.h"
#include "CLogger.h"
#include "stdio.h"
#include <string.h>
#include <vector>
#include <time.h>
#include <windows.h>
#include <ipexport.h>
#include <stdlib.h>
#include <sys/types.h>
#include <assert.h>
#include <fcntl.h>

#define MAXINTERFACES 16
#define MAC_ADDR_LEN 18
#define ADDR_LEN 16

/* begin change by duzf */
//#define rdtsc(low,high) __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))
/* end change by duzf */

struct timezone
{
	int  tz_minuteswest; /* minutes W of Greenwich */
	int  tz_dsttime;     /* type of dst correction */
};

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

int gettimeofday(struct timeval *tv, struct timezone *tz);

signed long long getElapsedRealtime();

signed long long currentTimebMillis();

namespace utility
{
	inline std::string addr_ntoa(uint32_t ip)
	{ 
		struct in_addr addr;
		memcpy(&addr, &ip, 4);
		return std::string(::inet_ntoa(addr)); 
	}

	inline std::string addr_ntoa_ipport(uint64_t ipport)
	{
		std::ostringstream os;
		struct in_addr addr;
		uint32_t ip = ipport >> 32;
		memcpy(&addr, &ip, 4);
		uint32_t port = 0x00000000FFFFFFFFLL & ipport;
		os << ::inet_ntoa(addr) << "-" << port;
		return os.str();
	}
	inline bool checkIp(uint32_t ip)
	{
		if((ip & 0x000000ff) == 10)           // 10.0.0.0~10.255.255.255
			return false;
		if((ip & 0x0000ffff) == 0xa8c0)       // 192.168.0.0~192.168.255.255
			return false;
		uint32_t a = ip & 0x000000ff;
		uint32_t b = (ip&0x0000ff00) >> 8;
		if(a == 0xac && b >= 16 && b <= 31) // 172.16.0.0~172.31.255.255
			return false;
		return true;
	}
	inline uint64_t ipPort2Uint64(uint32_t ip, uint32_t port)
	{
		return ((uint64_t)ip << 32) | (port & 0x00000000FFFFFFFFLL);
	}
	inline void uint64toIpPort(uint64_t id  ,Address& addr)
	{
		uint64_t tmp = id;
		addr.addrStruct.ip = uint32_t (id >> 32);
		addr.addrStruct.port = uint32_t (tmp & 0x00000000FFFFFFFFLL);
	}

	inline int getLocalIP(std::vector<uint32_t>& ips)
	{
		WSADATA	wd;
		char	buf[256];
		int		nRet;
		struct hostent    *phe;
		struct in_addr    *paddr;

		uint32_t loIp = inet_addr("127.0.0.1");
		nRet = WSAStartup(MAKEWORD(1, 1), &wd);
		if(nRet != 0){
			return 0;
		}
		if(gethostname(buf, 256) != SOCKET_ERROR){
			phe = gethostbyname(buf);
			if(phe != 0){
				while(phe->h_addr_list[nRet]){
					paddr = reinterpret_cast<struct in_addr*>(phe->h_addr_list[nRet++]);
					uint32_t localIp = paddr->S_un.S_addr;
					if(localIp != loIp){
						ips.push_back(localIp);
					}
				}
			}
		}
		WSACleanup();
		return nRet;
	}

	inline uint32_t getLocalIP()
	{
		WSADATA	wd;
		char	buf[256];
		int		nRet;
		struct hostent    *phe;
		struct in_addr    *paddr;

		uint32_t loIp = inet_addr("127.0.0.1");
		uint32_t localIp = 0;
		nRet = WSAStartup(MAKEWORD(1, 1), &wd);
		if(nRet != 0){
			return 0;
		}
		if(gethostname(buf, 256) != SOCKET_ERROR){
			phe = gethostbyname(buf);
			if(phe != 0){
				while(phe->h_addr_list[nRet]){
					paddr = reinterpret_cast<struct in_addr*>(phe->h_addr_list[nRet++]);
					if(paddr->S_un.S_addr != loIp){
						if(localIp != 0){
							return 0xffffffff;
						}
						localIp = paddr->S_un.S_addr;
					}
				}
			}
		}
		WSACleanup();
		return localIp;
	}

	inline uint64_t get_cpu_cycle()
	{
		/* begin change by duzf */
#if 0
		union cpu_cycle{
			struct t_i32 {
				uint32_t l;
				uint32_t h;
			} i32;
			uint64_t t;
		} c;

		rdtsc( c.i32.l, c.i32.h );

		return c.t;
#endif
		return 0;
		/* end change by duzf */
	}

	uint64_t get_usec_interval( uint64_t start, uint64_t stop );
	uint32_t get_msec_interval( uint64_t start, uint64_t stop );
	int init_cpu_freq();

}

struct ConfigNode
{
	union config_union_t 
	{
		struct counter_t {
			uint32_t max;
			uint32_t counter;
		};

		counter_t counter;
		bool flag;
	};

	config_union_t configData;

	ConfigNode(uint32_t m, uint32_t c)
	{
		configData.counter.max = m;
		configData.counter.counter = c;
	};

	ConfigNode(bool flag_)
	{
		configData.flag = flag_;
	};
};
typedef std::map<uint32_t, ConfigNode *> ConfigChecker_t;

class Configer
{
public:
	enum
	{
		MEDIA_LOGIN_PER_SEC = 1,//检查每一秒最多media用户登陆数
		MEDIA_USER_MAX      = 4,//检查此前端最多media用户数
		disablePMservice    = 10,//禁止前端管理服务
	};
	Configer();
	virtual ~Configer();

	static void addConfig(uint32_t key, ConfigNode* node);
	static void setConfigMax(uint32_t key, uint32_t max);
	static uint32_t getConfigMax(uint32_t key);
	static bool checkCounterPerSec(uint32_t key, uint32_t& count);
	static bool checkTotalMax(uint32_t key, uint32_t number);
	static void reSetAllCounter();
	static std::string getHelp();
	static void checkCounterRollback(uint32_t key);
	static bool check(uint32_t key);
	static void setConfig(uint32_t key, bool flag);
private:
	static ConfigChecker_t configChecker;
};

class timeMeasure 
{
public:
	timeMeasure() 
	{
		gettimeofday(&stv, &stz);
		startTime = stv.tv_usec + stv.tv_sec * 1000000;
	}

	~timeMeasure() {}

	long getTime() 
	{
		gettimeofday(&stv, &stz);
		return (stv.tv_usec + stv.tv_sec * 1000000) - startTime;
	}
private:
	long    startTime;
	struct timeval stv;
	struct timezone stz;
};

// 序号包异常检测
class SerialChecker
{
public:
	SerialChecker() : m_contBrkCntTolerance(10), m_contBrkTimeTolerance(300), m_firstBreakTime(0), m_breakCount(0) {}
	virtual ~SerialChecker() {}
	void init(uint32_t count, uint32_t interval) { m_contBrkCntTolerance = count; m_contBrkTimeTolerance = interval; }
	void reset() { m_firstBreakTime = 0; m_breakCount = 0; }
	//‘最大容忍连续乱序次数’以及‘最大容忍连续乱序时间’两个条件同时达到时才确定序号包异常
	bool check(uint32_t now);

private:
	// 最大容忍连续乱序次数
	uint32_t m_contBrkCntTolerance;
	// 最大容忍连续乱序时间
	uint32_t m_contBrkTimeTolerance;

	// 序号包第一次出现乱序时间
	uint32_t m_firstBreakTime;
	// 序号包连续出现乱序次数
	uint32_t m_breakCount;
};

namespace string_util
{
	enum
	{
		max_line_size = 1024
	};
	int getline(std::string& stream, std::string& line, uint32_t npos = 0);
	std::vector<std::string> split(const std::string &cmd);

	class Command
	{
	public:
		Command(const std::string& cmd);
		virtual ~Command();

		const std::string getCommand();
		const int getParamsNum() const {return m_vecParams.size();};
		const std::string getParamAsString(int index);
		int getParamAsInt(int index);
		uint32_t getParamAsUint(int index);
		bool getParamAsBool(int index);

	private:
		std::string m_strCmd;
		std::vector<std::string> m_vecParams;
	};
};

class StringUtil
{
public:
	static void split(std::string& s, std::string& delim, std::vector< std::string >* ret);
	static std::string trimLeft(const std::string& str);
	static std::string trimRight(const std::string& str);
	static std::string trim(const std::string& str);
	char* DumpHex(void* BinBuffer,char* pDumpBuffer,unsigned int DumpBufferLen);
};

#define NELEMENT(arr) (sizeof(arr)/sizeof(arr[0]))

//////////////////////////////////////////////////////////////////////////
static bool isNum(const char* start, const char* end) {
	for(const char* s = start; s!=end; ++s) {
		if(*s < '0' ||  *s > '9')
			return false;
	}
	return true;
}

static bool isInetAddr(const char* s) 
{
	const char* start = s;
	const char* end = strchr(s, '.');
	if(!end)
		return false;
	if(!isNum(start, end))
		return false;

	start = end + 1;
	end =  strchr(start, '.');
	if(!isNum(start, end))
		return false;

	start = end + 1;
	end =  strchr(start, '.');
	if(!isNum(start, end))
		return false;

	start = end + 1;
	end =  &s[strlen(s)];
	if(!isNum(start, end))
		return false;

	return true;
}

/** 
	errorCode 返回错误代码, 如果成功则返回0, 如果hostName==null则返回-1, 其他错误:
	WSANOTINITIALISED A successful WSAStartup must occur before using this function. 
	WSAENETDOWN The network subsystem has failed. 
	WSAHOST_NOT_FOUND Authoritative Answer Host not found. 
	WSATRY_AGAIN Non-Authoritative Host not found, or server failure. 
	WSANO_RECOVERY Nonrecoverable error occurred. 
	WSANO_DATA Valid name, no data record of requested type. 
	WSAEINPROGRESS A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. 
	WSAEFAULT The name parameter is not a valid part of the user address space. 
	WSAEINTR A blocking Windows Socket 1.1 call was canceled through WSACancelBlockingCall. 
*/
static std::vector<uint32_t> getHostByName(const char* hostName, long* errorCode = NULL) 
{
	long tempErrorCode = 0;
	errorCode = errorCode ? errorCode : &tempErrorCode;
	*errorCode = 0;

	std::vector<uint32_t> rt;
	if(!hostName) {
		*errorCode = -1;
		return rt;
	}

	if(isInetAddr(hostName)) {
		rt.push_back(inet_addr(hostName));	
		return rt;
	} else {
		WSADATA	wd;
		int nRet = WSAStartup(MAKEWORD(1, 1), &wd);
		if(nRet != 0) {
			*errorCode = WSAGetLastError();	
			log(Error, "%s WSAStartup failed, error=%u", __FUNCTION__, *errorCode);
			return rt;
		}

		HOSTENT* host_entry=gethostbyname(hostName);
		if(host_entry == NULL) {
			*errorCode = WSAGetLastError();
			log(Error, "%s failed,error=%u", __FUNCTION__, *errorCode);
		} else {
			if(host_entry->h_addrtype == AF_INET)
			{		
				for(int i=0; host_entry->h_addr_list[i] != NULL; ++i) {
					uint32_t ip = 0;
					memcpy(&ip, host_entry->h_addr_list[i], host_entry->h_length);
					rt.push_back(ip);
				}
			}
		}

		WSACleanup();
	}

	return rt;
}

enum {
	LITTLE_ENDIAN = 0,
	BIG_ENDIAN,
};

static const uint32_t __endianTest = 1;
static const uint16_t*  __lowword_ptr = (const uint16_t*)&__endianTest;
#define IS_BIG_ENDIAN() (__endianTest != *__lowword_ptr)

__inline void memrev(uint8_t* dst, const uint8_t* src, int count) 
{
	for(int i=0; i<count; ++i) {
		dst[count-i-1] = src[i];
	}
}

template <typename T>
static __inline int putValue(void* buf, const T& v) {
	memcpy((uint8_t*)buf, &v, sizeof(v));
	return sizeof(T);
}

template <typename T>
static __inline int putValue_le(void* buf, const T& v) {
	if(IS_BIG_ENDIAN()) {
		memrev((uint8_t*)buf, (const uint8_t*)&v, sizeof(v));
	} else {
		memcpy((uint8_t*)buf, &v, sizeof(v));
	}
	return sizeof(T);
}

template <typename T>
static __inline int putValue_be(void* buf, const T& v) {
	if(IS_BIG_ENDIAN()) {
		memcpy((uint8_t*)buf, &v, sizeof(v));
	} else {
		memrev((uint8_t*)buf, (const uint8_t*)&v, sizeof(v));	
	}
	return sizeof(T);
}

template <typename T>
__inline int getValue_le(T* v, const void* buf) 
{
	if(IS_BIG_ENDIAN()) {
		memrev((uint8_t*)v, (const uint8_t*)buf, sizeof(T));	
	} else {
		memcpy(v, buf, sizeof(T));
	}
	return sizeof(T);
}

template <typename T>
__inline int getValue_be(T* v, const void* buf) 
{
	if(IS_BIG_ENDIAN()) {
		memcpy(v, buf, sizeof(T));
	} else {
		memrev((uint8_t*)v, (const uint8_t*)buf, , sizeof(T));	
	}
	return sizeof(T);
}

template <typename T>
static __inline int getValue(T* v, const void* buf) {
	memcpy(v, (const uint8_t*)buf, sizeof(T));
	return sizeof(T);
}

/**
	@ 以路径方式连接两个字符串
	如: pathCat("c:\a","b\c")=c:\a\b\c
	返回: 成功则返回连接后的路径,否则返回""
*/
std::string pathCat(const char* p1, const char* p2);

/**
	@ 获取一个路径的父路径
	如:pathGetParent("c:\a")=c:\
	返回: 成功则返回路径的父路径,否则返回""
*/
std::string pathGetParent(const char* path);

/**
	@ 把路径p转换为绝对路径,如果path已经为一个绝对路径,则直接返回path
	如: 相对路径为c:\a,pathConvToAbsolute("b\c")=c:a\b\c,
	pathConvToAbsolute("c:\a\b\c")=c:\a\b\c
	返回: 如果成功则返回转换后的绝对路径,否则返回""
*/
std::string pathConvToAbsolute(const char* path);

/** 
	@ 判断路径path是否一个根路径
	如: pathIsRoot("c:\")=true,	pathIsRoot("\")=true, pathIsRoot("c:\a")=false
	返回: 指定路径是根路径则返回true,否则返回false
*/
bool pathIsRoot(const char* path);

/** 
	@ 返回当前可执行文件所在路径
*/
std::string getCwd();

#define RETURN_IF(exp, rt) do { if(exp) return (rt); } while(0)

static void stringToVector(std::vector<unsigned char>& vec, const std::string& s) 
{
	vec.resize(s.length());
	memcpy(&vec[0], &s[0], s.length());
}

inline int64_t currentTimeMillis()
{
	timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

unsigned long getTickCount(void);



#endif
