#ifndef CLOGGER_H
#define CLOGGER_H

#include <string>
#include <stdio.h>
#include "tinythread.h"

#define Log (*Logger::getInstance())
#define log(loglevel, fmt, ...) do {Log.writelog(loglevel, fmt, ## __VA_ARGS__);} while (0)
/*
#define logEmerg(fmt, ...)	loggerPtr->writelog(LOG_EMERG, __FILE__, __LINE__, __FUNCTION__, fmt, ## __VA_ARGS__)
#define logAlert(fmt, ...)	loggerPtr->writelog(LOG_ALERT, __FILE__, __LINE__, __FUNCTION__, fmt, ## __VA_ARGS__)
#define logCrit(fmt, ...)	loggerPtr->writelog(LOG_CRIT, __FILE__, __LINE__, __FUNCTION__, fmt, ## __VA_ARGS__)
#define logError(fmt, ...)	loggerPtr->writelog(LOG_ERR, __FILE__, __LINE__, __FUNCTION__, fmt, ## __VA_ARGS__)
#define logWarn(fmt, ...)	loggerPtr->writelog(LOG_WARNING, __FILE__, __LINE__, __FUNCTION__, fmt, ## __VA_ARGS__)
#define logNotice(fmt, ...)	loggerPtr->writelog(LOG_NOTICE, __FILE__, __LINE__, __FUNCTION__, fmt, ## __VA_ARGS__)
#define logInfo(fmt, ...)	loggerPtr->writelog(LOG_INFO, __FILE__, __LINE__, __FUNCTION__, fmt, ## __VA_ARGS__)
#define logDebug(fmt, ...)	loggerPtr->writelog(LOG_DEBUG, __FILE__, __LINE__, __FUNCTION__, fmt, ## __VA_ARGS__)
*/

enum LogLevel{
	Fatal,
	Alert,
	Crit,
	Error,
	Warn,
	Notice,
	Info,
	Debug,
	Verbose
};

class Logger
{
public:
	Logger(void);
	~Logger(void);

	static Logger* getInstance()
	{
		if (NULL == m_instance)
		{
			m_instance = new Logger;
		}
		return m_instance;
	}

	inline std::string time();

	void em(const char *tag, const char *fmt, ...);

	void a(const char *tag, const char *fmt, ...);

	void c(const char *tag, const char *fmt, ...);

	void e(const char *tag, const char *fmt, ...);

	void w(const char *tag, const char *fmt, ...);

	void n(const char *tag, const char *fmt, ...);

	void i(const char *tag, const char *fmt, ...);

	void d(const char *tag, const char *fmt, ...);

	void v(const char *tag, const char *fmt, ...);


	void writelog(LogLevel debug_level, const char *fmt, ...);

	static const char *TAG_SVC;
	static const char *TAG_NETWORK;
	static const char *TAG_LINKD;
	static const char *TAG_LBS;
	static const char *TAG_CALL;
	static const char *TAG_FOR_ALL;
	static const char *TAG_GROUP;

	enum Log_Level
	{
		LOG_EMERG,	/* system is unusable */
		LOG_ALERT,	/* action must be taken immediately */
		LOG_CRIT,	/* critical conditions */
		LOG_ERR,	/* error conditions */
		LOG_WARNING,/* warning conditions */
		LOG_NOTICE,	/* normal but significant condition */
		LOG_INFO,	/* informational */
		LOG_DEBUG,	/* debug-level messages */
		LOG_VERBOSE,
		MAX_LOG_LEVEL
	};

private:

	static Logger *m_instance;
	char logdata[1024];
	Log_Level m_loglevel;

	typedef tthread::recursive_mutex Mutex;
	Mutex mMutex;
};


char *ip2str(unsigned int ip);
char *uri2str(unsigned int uri);

#if defined(__x86_64__)
char *int642str(unsigned long num);
#else
char *int642str(unsigned long long num);
#endif
char *int2str(unsigned int num);
char *uint2str(unsigned int num);

#endif // CLOGGER_H
