#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <string>
#include <ctime>
#include <stdexcept>
#include <map>
#include <set>
#include <queue>
#include <sstream>
#include <vector>

using namespace std;

typedef unsigned int URI_TYPE;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;

#ifdef __x86_64__
typedef signed long int64_t;
typedef unsigned long uint64_t;
#else
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
#endif

union Address {
	uint64_t ipport;
	struct {
		uint32_t ip;
		uint32_t port;
	} addrStruct;
};

typedef uint8_t  jbyte;
typedef int16_t jshort;
typedef int32_t jint;
typedef bool    jboolean;
typedef bool    jbool;
typedef int64_t jlong;

#endif
