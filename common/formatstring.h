#ifndef __FormatString_H
#define __FormatString_H

#include <tchar.h>
#include <stdio.h>
#include <string>

#if defined(_UNICODE) || defined(UNICODE)
#define tstring std::wstring
#define formatString formatStringW
#else
#define tstring std::string
#define formatString formatStringA
#endif

static std::string formatStringA(const char* fmt, ...) 
{
	char buf[512] = "";
	va_list va;
	va_start(va, fmt);
	vsprintf_s(buf, sizeof(buf), fmt, va);
	va_end(va);
	return buf;
}

static std::wstring formatStringW(const wchar_t* fmt, ...) 
{
	wchar_t buf[512] = L"";
	va_list va;
	va_start(va, fmt);
	vswprintf_s(buf, sizeof(buf), fmt, va);
	va_end(va);
	return buf;
}

#endif //__FormatString_H