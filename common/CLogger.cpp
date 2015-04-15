#include "CLogger.h"
#include <stdarg.h>
#include <time.h>
#include <io.h>

// uncomment the next line to enable macro definition, if timestamp is needed in log
#define LOGGER_NEED_TIMESTAMP

Logger *Logger::m_instance = NULL;

const char *Logger::TAG_SVC = "yysdk-svc";
const char *Logger::TAG_NETWORK = "yysdk-network";
const char *Logger::TAG_LINKD = "yysdk-linkd";
const char *Logger::TAG_LBS = "yysdk-lbs";
const char *Logger::TAG_CALL = "yysdk-call";
const char *Logger::TAG_FOR_ALL = "yysdk";
const char *Logger::TAG_GROUP = "yysdk-group";

enum TEXT_COLOR
{
	TXT_RED,
	TXT_GREEN,
	TXT_BLUE,
	TXT_YELLOW, // R+G
	TXT_PURPLE, // R+B
	TXT_CYAN,   // B+G
	TXT_WHITE,  // R+G+B
	TXT_BLACK,  
	TXT_GRAY,
	TXT_DEFAULT = TXT_WHITE,
};
#ifdef _WIN32
#include <Windows.h>
static WORD toBkColor(TEXT_COLOR c) 
{
	WORD rt = 0;
	switch(c) {
	case TXT_RED:
		rt = FOREGROUND_RED|FOREGROUND_INTENSITY;
		break;
	case TXT_GREEN:
		rt = FOREGROUND_GREEN|FOREGROUND_INTENSITY;
		break;
	case TXT_BLUE:
		rt = FOREGROUND_BLUE|FOREGROUND_INTENSITY;
		break;
	case TXT_YELLOW:
		rt = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		break;
	case TXT_PURPLE:
		rt = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
		break;
	case TXT_CYAN:
		rt = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		break;
	case TXT_WHITE:
		rt = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
		break;
	case TXT_GRAY:
		rt = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
		break;
	default:
		rt = 0;
		break;
	}
	return rt;
}

static TEXT_COLOR toTxtColor(WORD fgclr) 
{
	TEXT_COLOR rt = TXT_BLACK;

	switch(fgclr&0x7) {
	case FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED:
		if(fgclr & FOREGROUND_INTENSITY)
			rt = TXT_WHITE;
		else 
			rt = TXT_GRAY;
		break;
	case FOREGROUND_RED | FOREGROUND_GREEN:
		rt = TXT_YELLOW;
		break;
	case FOREGROUND_RED | FOREGROUND_BLUE:
		rt = TXT_PURPLE;
		break;
	case FOREGROUND_BLUE | FOREGROUND_GREEN:
		rt = TXT_CYAN;
		break;
	case FOREGROUND_RED:
		rt = TXT_RED;
		break;
	case FOREGROUND_GREEN:
		rt = TXT_GREEN;
		break;
	case FOREGROUND_BLUE:
		rt = TXT_BLUE;
		break;
	case 0:
		rt = TXT_BLACK;
		break;
	default:
		rt = TXT_WHITE;
		break;
	}
	return rt;
}


static unsigned short getConsoleAttributes()
{
	HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO bufInfo = {0};
	if(!GetConsoleScreenBufferInfo(hstdout, &bufInfo))
		return 0;
	return bufInfo.wAttributes;
}

static bool setTextColor(TEXT_COLOR clr)
{ 
	HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
	return SetConsoleTextAttribute(hstdout, getConsoleAttributes() & ~0x000f | toBkColor(clr)) ? true : false; 
}; 

static TEXT_COLOR getTextSolor()
{
	return toTxtColor(getConsoleAttributes() & 0x000f);
}

#else
static bool SetTextColor(TEXT_COLOR clr) 
{
	return false;
}
static TEXT_COLOR getTextSolor()
{
	return TXT_DEFAULT;
}
#endif

static bool fpisatty(FILE* fp)
{
	if(!fp)
		return false;
	return _isatty(_fileno(fp)) ? true : false;
}


class C
{
public:
	C(TEXT_COLOR c)
	{
		if(fpisatty(stdout))
			setTextColor(c); 
	}
	
	~C() 
	{
		if(fpisatty(stdout))
			setTextColor(TXT_WHITE);
	}
};

const char LogPriorityName[][8] =
{
    "EMERG",
    "ALERT",
    "CRIT",
    "ERROR",
    "WARNING",
    "NOTICE",
    "INFO",
    "DEBUG",
	"VERBOSE"
};

Logger::Logger()
{
	m_loglevel = LOG_VERBOSE;
}

Logger::~Logger(void)
{
}

std::string Logger::time()
{
#ifdef LOGGER_NEED_TIMESTAMP
	/*
	time_t rawtime;
	struct tm * timeinfo;
	::time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	const char* strTime = asctime(timeinfo);
	return std::string(strTime, strlen(strTime) - 1);
	*/
	SYSTEMTIME stTime;
	GetLocalTime(&stTime);
	char szDate[128] = {0};
	_snprintf_s(szDate, _countof(szDate), _TRUNCATE, "%d-%d-%d %d:%d:%d %dms",
		stTime.wYear, 
		stTime.wMonth,
		stTime.wDay,
		stTime.wHour,
		stTime.wMinute,
		stTime.wSecond,
		stTime.wMilliseconds);
	return szDate;
#else
	return "";
#endif
}

TEXT_COLOR loglv2TxtClr(int level) 
{
	TEXT_COLOR c;
	switch(level) {
	case Logger::LOG_EMERG:
	case Logger::LOG_ALERT:
	case Logger::LOG_CRIT:
	case Logger::LOG_ERR:
		c = TXT_RED;
		break;
	case Logger::LOG_WARNING:
		c = TXT_PURPLE;
		break;
	case Logger::LOG_NOTICE:
	case Logger::LOG_INFO:
		c = TXT_YELLOW;
		break;
	case Logger::LOG_DEBUG:
		c = TXT_WHITE;
		break;
	case Logger::LOG_VERBOSE:
		c = TXT_GRAY;
		break;
	default:
		c = TXT_WHITE;
		break;
	}
	return c;
}

void Logger::em(const char *tag, const char *fmt, ...)
{
	tthread::lock_guard<Mutex> guard(mMutex);
	if (LOG_EMERG > m_loglevel)
	{
		return;
	}

    va_list arg;
    va_start(arg, fmt);
    vsprintf_s(logdata, sizeof(logdata), fmt, arg);
    va_end(arg);

	C c(loglv2TxtClr(LOG_EMERG));
	printf("%s [%s] (%s) %s\n", time().c_str(), LogPriorityName[LOG_EMERG], tag, logdata);
	fflush(stdout);
}

void Logger::a(const char * tag, const char *fmt, ...)
{
	tthread::lock_guard<Mutex> guard(mMutex);
	if (LOG_ALERT > m_loglevel)
	{
		return;
	}

    va_list arg;
    va_start(arg, fmt);
    vsprintf_s(logdata, sizeof(logdata), fmt, arg);
    va_end(arg);

	C c(loglv2TxtClr(LOG_ALERT));
	printf("%s [%s] (%s) %s\n", time().c_str(), LogPriorityName[LOG_ALERT], tag, logdata);
	fflush(stdout);
}

void Logger::c(const char * tag, const char *fmt, ...)
{
	tthread::lock_guard<Mutex> guard(mMutex);
	if (LOG_CRIT > m_loglevel)
	{
		return;
	}

    va_list arg;
    va_start(arg, fmt);
    vsprintf_s(logdata, sizeof(logdata), fmt, arg);
    va_end(arg);

	C c(loglv2TxtClr(LOG_CRIT));
	printf("%s [%s] (%s) %s\n", time().c_str(), LogPriorityName[LOG_CRIT], tag, logdata);
	fflush(stdout);
}

void Logger::e(const char * tag, const char *fmt, ...)
{
	tthread::lock_guard<Mutex> guard(mMutex);
	if (LOG_ERR > m_loglevel)
	{
		return;
	}

    va_list arg;
    va_start(arg, fmt);
    vsprintf_s(logdata, sizeof(logdata), fmt, arg);
    va_end(arg);

	C c(loglv2TxtClr(LOG_ERR));
	printf("%s [%s] (%s) %s\n", time().c_str(), LogPriorityName[LOG_ERR], tag, logdata);
	fflush(stdout);
}

void Logger::w(const char * tag, const char *fmt, ...)
{
	tthread::lock_guard<Mutex> guard(mMutex);
	if (LOG_WARNING > m_loglevel)
	{
		return;
	}

    va_list arg;
    va_start(arg, fmt);
    vsprintf_s(logdata, sizeof(logdata), fmt, arg);
    va_end(arg);

	C c(loglv2TxtClr(LOG_WARNING));
	printf("%s [%s] (%s) %s\n", time().c_str(), LogPriorityName[LOG_WARNING], tag, logdata);
	fflush(stdout);
}

void Logger::n(const char * tag, const char *fmt, ...)
{
	tthread::lock_guard<Mutex> guard(mMutex);
	if (LOG_NOTICE > m_loglevel)
	{
		return;
	}

    va_list arg;
    va_start(arg, fmt);
    vsprintf_s(logdata, sizeof(logdata), fmt, arg);
    va_end(arg);

	C c(loglv2TxtClr(LOG_NOTICE));
	printf("%s [%s] (%s) %s\n", time().c_str(), LogPriorityName[LOG_NOTICE], tag, logdata);
	fflush(stdout);
}

void Logger::i(const char * tag, const char *fmt, ...)
{
	tthread::lock_guard<Mutex> guard(mMutex);
	if (LOG_INFO > m_loglevel)
	{
		return;
	}

    va_list arg;
    va_start(arg, fmt);
    vsprintf_s(logdata, sizeof(logdata), fmt, arg);
    va_end(arg);

	C c(loglv2TxtClr(LOG_INFO));
	printf("%s [%s] (%s) %s\n", time().c_str(), LogPriorityName[LOG_INFO], tag, logdata);
	fflush(stdout);
}

void Logger::d(const char * tag, const char *fmt, ...)
{
	tthread::lock_guard<Mutex> guard(mMutex);
	if (LOG_DEBUG > m_loglevel)
	{
		return;
	}

    va_list arg;
    va_start(arg, fmt);
    vsprintf_s(logdata, sizeof(logdata), fmt, arg);
    va_end(arg);

	C c(loglv2TxtClr(LOG_DEBUG));
	printf("%s [%s] (%s) %s\n", time().c_str(), LogPriorityName[LOG_DEBUG], tag, logdata);
	fflush(stdout);
}

void Logger::v(const char * tag, const char *fmt, ...)
{
	tthread::lock_guard<Mutex> guard(mMutex);
	if (LOG_VERBOSE > m_loglevel)
	{
		return;
	}

    va_list arg;
    va_start(arg, fmt);
    vsprintf_s(logdata, sizeof(logdata), fmt, arg);
    va_end(arg);

	C c(loglv2TxtClr(LOG_VERBOSE));
	printf("%s [%s] (%s) %s\n", time().c_str(), LogPriorityName[LOG_VERBOSE], tag, logdata);
	fflush(stdout);
}

void Logger::writelog(LogLevel debug_level, const char *fmt, ...)
{
	tthread::lock_guard<Mutex> guard(mMutex);
	if (debug_level > m_loglevel)
	{
		return;
	}

    va_list arg;
    va_start(arg, fmt);
    vsprintf_s(logdata, sizeof(logdata), fmt, arg);
    va_end(arg);

	C c(loglv2TxtClr(debug_level));
	printf("%s [%s] %s\n", time().c_str(), LogPriorityName[debug_level], logdata);
	fflush(stdout);
}

char *ip2str(unsigned int ip) {
	union ip_addr {
		unsigned int addr;
		unsigned char s[4];
	} a;
	a.addr = ip;
	static char s[32];
	sprintf_s(s, 32, "%u.%u.%u.%u", a.s[0], a.s[1], a.s[2], a.s[3]);
	return s;
}

char *uri2str(unsigned int uri) {
	unsigned int uri_h = uri / 256;
	unsigned int uri_t = uri % 256;

	static char s[32];
	sprintf_s(s, 32, "%3d|%3d", uri_h, uri_t);
	return s;
}

char *int2str(unsigned int num) {
	static char s[24];
	sprintf_s(s,24, "%d", num);
	return s;
}

char *uint2str(unsigned int num) {
	static char s[24];
	sprintf_s(s,24, "%u", num);
	return s;
}

#if defined(__x86_64__)
char *int642str(unsigned long num)
#else
char *int642str(unsigned long long num)
#endif
{
	static char s[32];
#if defined(__x86_64__)
	sprintf_s(s, 32, "%ld", num);
#else
	sprintf_s(s, 32, "%lld", num);
#endif
	return s;
}