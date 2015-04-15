#include "utility.h"
/* begin change by duzf */
#include <iostream>
#include <iterator>
/* end change by duzf */
#include <ctype.h>
#include "filepath.h"

#include <sys/timeb.h>

using namespace std;
static double cpu_freq = 1; /*MHz*/
static double cpu_freq_magnification = 1; /*MHz*/

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	// Define a structure to receive the current Windows filetime
	FILETIME ft;

	// Initialize the present time to 0 and the timezone to UTC
	unsigned __int64 tmpres = 0;
	static int tzflag = 0;

	if (NULL != tv)
	{
		GetSystemTimeAsFileTime(&ft);

		// The GetSystemTimeAsFileTime returns the number of 100 nanosecond 
		// intervals since Jan 1, 1601 in a structure. Copy the high bits to 
		// the 64 bit tmpres, shift it left by 32 then or in the low 32 bits.
		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		// Convert to microseconds by dividing by 10
		tmpres /= 10;

		// The Unix epoch starts on Jan 1 1970.  Need to subtract the difference 
		// in seconds from Jan 1 1601.
		tmpres -= DELTA_EPOCH_IN_MICROSECS;

		// Finally change microseconds to seconds and place in the seconds value. 
		// The modulus picks up the microseconds.
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (long)(tmpres % 1000000UL);
	}

	if (NULL != tz)
	{
		if (!tzflag)
		{
			_tzset();
			tzflag++;
		}

		// Adjust for the timezone west of Greenwich
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}

	return 0;
}

signed long long getElapsedRealtime()
{
	struct timeval stv;
	gettimeofday(&stv, NULL);
	return ((signed long long)stv.tv_sec)*1000 + stv.tv_usec/1000;
}

signed long long currentTimebMillis()
{
	struct timeb tp;
	ftime(&tp);
	return (tp.time * 1000 + tp.millitm);
}

//////////////////////////////////////////////////////////////////////////
//  Configer
//////////////////////////////////////////////////////////////////////////
ConfigChecker_t Configer::configChecker;

Configer::Configer()
{
  ConfigNode* secMediaLogin = new ConfigNode(500, 0);
  addConfig(MEDIA_LOGIN_PER_SEC, secMediaLogin);
  ConfigNode* mediaUserCount = new ConfigNode(4000, 0);
  addConfig(MEDIA_USER_MAX, mediaUserCount);

  ConfigNode* pConfigNode = new ConfigNode(false);
  addConfig(disablePMservice, pConfigNode);
}

Configer::~Configer()
{
  ConfigChecker_t::iterator it = configChecker.begin();
  for(; it != configChecker.end(); it++)
    delete  it->second;
  configChecker.clear();
}

void Configer::addConfig(uint32_t key, ConfigNode* node)
{
  configChecker[key] = node;
}

bool Configer::check(uint32_t key)
{
  ConfigChecker_t::iterator it = configChecker.find(key);
  if(it == configChecker.end())
    return false;

  return it->second->configData.flag;
}

void Configer::setConfig(uint32_t key, bool flag)
{
  ConfigChecker_t::iterator it = configChecker.find(key);
  if(it != configChecker.end())
    it->second->configData.flag = flag;
}

bool Configer::checkCounterPerSec(uint32_t key, uint32_t& counter)
{
  counter = 0;
  ConfigChecker_t::iterator it = configChecker.find(key);
  if (it != configChecker.end())
  {
    counter = it->second->configData.counter.counter++;
    if( it->second->configData.counter.counter <= it->second->configData.counter.max )
      return true;
  }
  return false; 
}

bool Configer::checkTotalMax(uint32_t key, uint32_t number)
{
  ConfigChecker_t::iterator it = configChecker.find(key);
  if (it != configChecker.end() && number <= it->second->configData.counter.max )
    return true;

  return false; 
}

void Configer::setConfigMax(uint32_t key, uint32_t max)
{
  ConfigChecker_t::iterator it = configChecker.find(key);
  if (it != configChecker.end())
    it->second->configData.counter.max = max;
}
uint32_t Configer::getConfigMax(uint32_t key)
{
  uint32_t value = 0;
  ConfigChecker_t::iterator it = configChecker.find(key);
  if (it != configChecker.end())
    value = it->second->configData.counter.max ;

  return value;
}


void Configer::reSetAllCounter()
{
  ConfigChecker_t::iterator it = configChecker.begin();
  for(; it != configChecker.end(); it++)
    it->second->configData.counter.counter = 0;
}

void Configer::checkCounterRollback(uint32_t key)
{
  ConfigChecker_t::iterator it = configChecker.find(key);
  if (it != configChecker.end())
  {
    if (it->second->configData.counter.counter > 0)
      it->second->configData.counter.counter--;
  }
}

std::string Configer::getHelp()
{
  std::ostringstream os;
  os << "set"  << std::endl;
  os << " mediaLoginMax/sec  1 "  << std::endl;
  os << " mediaUserMax       4 "  << std::endl;
  os << " lossRateThreshold  6 "  << std::endl;
  os << " log level          7 "  << std::endl;
  os << " disablePMservice   10 "   << std::endl;
  return os.str();
}

bool SerialChecker::check(uint32_t now)
{
  if (m_breakCount == 0)
  {
    m_firstBreakTime = now;
  }

  ++m_breakCount;

  if (m_breakCount < m_contBrkCntTolerance)
    return true;

  if ((m_firstBreakTime == 0) || (now < (m_firstBreakTime + m_contBrkTimeTolerance)))
    return true;

  return false;
}


namespace utility
{
  uint64_t get_usec_interval( uint64_t start, uint64_t stop )
  {
    if (stop < start) return 0;
    return (uint64_t)((stop - start) / cpu_freq);
  }

  uint32_t get_msec_interval( uint64_t start, uint64_t stop )
  {
    if (stop < start) return 0;
    return (uint32_t)((stop - start) / cpu_freq_magnification);
  }
/*
  int init_cpu_freq()
  {
    FILE * fp;
    char * str;
    const char * cmd;
    int ratio = 1;

    str = (char*)malloc(1024);

    fp = popen( "cat /proc/cpuinfo | grep -m 1 \"model name\"", "r" );
    fgets( str, 1024, fp );
    fclose( fp );

    if( strstr( str, "AMD" ) )
    {
      cmd =  "cat /proc/cpuinfo | grep -m 1 \"cpu MHz\" | sed -e \'s/.*:[^0-9]//\'";
    }
    else
    {
      cmd = "cat /proc/cpuinfo | grep -m 1 \"model name\" | sed -e \"s/^.*@ //g\" -e \"s/GHz//g\"";
      ratio = 1000;
    }

    fp = popen( cmd, "r" );
    if( fp == NULL )
    {
      log(Error, "get cpu info failed\n");
      return -1;
    }
    else
    {
      fgets( str, 1024, fp );
      cpu_freq = atof(str) * ratio;
      cpu_freq_magnification = cpu_freq*1000;
      fclose( fp );
      log(Info, "get cpu info cpu_freq %f cpu_freq_magnification %f ", cpu_freq, cpu_freq_magnification);
    }

    free( str );

    return 0;
  }
*/
}

vector<std::string> string_util::split(const string &cmd)
{
  vector<string> tokens;
  istringstream iss(cmd);
  copy(istream_iterator<string>(iss),
           istream_iterator<string>(),
           back_inserter<vector<string> >(tokens));
  return tokens;
}

int string_util::getline(std::string& stream, std::string& line, uint32_t npos)
{
  int nsize = 0;
  int i = 0;
  for (; (i < max_line_size) && (npos + i < stream.size()); ++i) {
    if (stream[i + npos] == '\n') {
      nsize = i + 1;
      break;
    }
    if (stream[i + npos] == '\r') {
      if (((i + npos + 1) < stream.size()) && stream[i + npos + 1] == '\n') {
        nsize = i + 2;
        break;
      } else {
        nsize = i + 1;
        break;
      }
    }
  }
  line = stream.substr(npos, i);
  return nsize;
}

string_util::Command::Command(const std::string& cmd)
{
  size_t npos = cmd.find_first_of(" ");
  m_vecParams.clear();

  if (npos == std::string::npos)
  {
    m_strCmd = cmd;
  }
  else
  {
    m_strCmd = cmd.substr(0, npos);
    string params = cmd.substr(npos + 1);

    m_vecParams = split(params);
  }
}

string_util::Command::~Command()
{

}

const std::string string_util::Command::getCommand()
{
  return m_strCmd;
}

const std::string string_util::Command::getParamAsString(int index)
{
  if (index >= (int)m_vecParams.size())
  {
    return "";
  }

  return m_vecParams[index];
}

int string_util::Command::getParamAsInt(int index)
{
  int intValue = atoi(getParamAsString(index).c_str());
  return intValue;
}

uint32_t string_util::Command::getParamAsUint(int index)
{
  uint32_t ret = 0;
  sscanf_s(getParamAsString(index).c_str(), "%u", &ret);
  return ret;
}

bool string_util::Command::getParamAsBool(int index)
{
  return (getParamAsInt(index) > 0);
}

//////////////////////////////////////////////////////////////////////////
//  StringUtil
//////////////////////////////////////////////////////////////////////////
void StringUtil::split(string& s, string& delim,std::vector<string>* ret)
{
    size_t last = 0;
    size_t index=s.find_first_of(delim,last);
    while (index!=std::string::npos)
    {
        ret->push_back(s.substr(last,index-last));
        last=index+1;
        index=s.find_first_of(delim,last);
    }
    if (index-last>0)
    {
        ret->push_back(s.substr(last,index-last));
    }
}

string StringUtil::trimLeft(const string& str)
{
    string t = str;
    for (string::iterator i = t.begin(); i != t.end(); i++)
    {
        if (!isspace(*i))
        {
            t.erase(t.begin(), i);
            break;
        }
    }
    return t;
}

string StringUtil::trimRight(const string& str)
{
    if (str.begin() == str.end())
    {
        return str;
    }

    string t = str;
    for(string::iterator i = t.end() - 1;;--i)
    {
        if (!isspace(*i))
        {
            t.erase(i + 1, t.end());
            break;
        }
        if(i == t.begin())
            return "";
    }
    return t;
}

string StringUtil::trim(const string& str)
{
    return trimRight(trimLeft(str));
}

static char HexTable[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

char* StringUtil::DumpHex(void* BinBuffer,char* pDumpBuffer,unsigned int DumpBufferLen)
{
    char* pTemp = pDumpBuffer;
    char* pSrc = (char*)BinBuffer;

    for(unsigned int i = 0;i < DumpBufferLen / 2;i++)
    {
        *pTemp = HexTable[pSrc[i] >> 4 & 0x0f];
        *++pTemp = HexTable[pSrc[i] & 0x0f];
        pTemp ++;
    }
    return pDumpBuffer;
}

#define IS_PATH_DELIMITER(c) ((c)=='\\'||(c)=='/')

string pathCat(const char* p1, const char* p2)
{
	string rt;

	if(!p1 && !p2)
		return rt;

	const size_t P1_LEN = strlen(p1);
	const size_t P2_LEN = strlen(p2);
	if(P1_LEN==0 && P2_LEN==0) {
		rt = "";
	} else if(P1_LEN==0 && P2_LEN != 0) {
		rt = p2;
	} else if(P1_LEN != 0 && P2_LEN == 0) {
		rt = p1;
	} else {
		const size_t DELIM_CNT = IS_PATH_DELIMITER(p1[P1_LEN-1]) + IS_PATH_DELIMITER(p2[0]);
		if(DELIM_CNT == 0) {
			rt = string() + p1 + "/" + p2;
		} else if(DELIM_CNT == 1) {
			rt = string() + p1 + p2;
		} else if(DELIM_CNT == 2) {
			rt = p1;
			rt.erase(P1_LEN-1);
			rt+=p2;
		}
	}

	if(IS_PATH_DELIMITER(rt[rt.length()-1]))
		rt.erase(rt.length()-1);

	return rt;	
}

#if defined(_WIN32)
#include <direct.h>
#define GETCWD _getcwd
#else
#include "unistd.h"
#define GETCWD getcwd
#endif // _WIN32

string getCwd() 
{
	char buf[MAX_PATH];
	if(!GETCWD(buf, MAX_PATH))
		return "";
	return buf;
}

bool pathIsRoot(const char* path) 
{
	if(!path)
		return false;
	string _path = path;
	if(_path == "/" || _path=="\\")
		return true;
#if defined(_WIN32)
	if(isalpha(_path[0]) && _path[1] == ':') {
		if(_path[2] == '\0')
			return true;
		if(IS_PATH_DELIMITER(_path[2]) && _path[3] == '\0')
			return true;
	}
#endif
	return false;
}

string pathGetParent(const char* path)
{
	if(!path || pathIsRoot(path))
		return "";

	int pos = strlen(path)-1;
	while(pos >= 0) {
		if(IS_PATH_DELIMITER(path[pos--])) {
			break;
		}
	}
	if(pos < 0)
		return "";

	string rt;
	rt.append(path, pos+1);

	return rt;
}

string pathConvToAbsolute(const char* path) 
{
	if(!path || strlen(path) < 1)
		return "";

	string rt;
	filepath filpath(path);
	if(filpath.is_absolute()) {
		rt = path;
	} else {
		string cwd = getCwd();
		if(cwd == "")
			return "";
		rt = pathCat(cwd.c_str(), path);
	}

	return rt;
}


#if defined(_WIN32)
#define OS_WINDOWS WIN32
#include <windows.h>
#endif
#if defined(LINUX) || defined(linux)
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>
#endif
#if defined(OS_VXWORKS)
#include "vxworks.h"
#include <tickLib.h>
#include <sysLib.h>
#endif

unsigned long getTickCount(void)
{
	unsigned long currentTime;
#if defined(_WIN32)
	currentTime = GetTickCount();
#endif
#if defined(LINUX) || defined(linux)
	struct timeval current;
	gettimeofday(&current, NULL);
	currentTime = current.tv_sec * 1000 + current.tv_usec/1000;
#endif
#if defined(OS_VXWORKS)
	unsigned long timeSecond = tickGet() / sysClkRateGet();
	unsigned long timeMilsec = tickGet() % sysClkRateGet() * 1000 / sysClkRateGet();
	currentTime = timeSecond * 1000 + timeMilsec;
#endif
	return currentTime;
}


//end
