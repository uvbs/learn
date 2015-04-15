#ifndef __SNOX_SOCKBUFFER_H_INCLUDE__
#define __SNOX_SOCKBUFFER_H_INCLUDE__

#include <stdexcept>
#include "SocketBase.h"
#include "blockbuffer.h"

class buffer_overflow: public std::runtime_error {
public:
	buffer_overflow(const std::string& _w) :
		std::runtime_error(_w) {
	}
};

class buffer_overflow_all: public buffer_overflow {
public:
	buffer_overflow_all(const std::string& _w) :
		buffer_overflow(_w) {
	}
};

class FilterDefault {
protected:
	void filterRead(char *, size_t) {
	}
	char *filterWrite(char *data, size_t) {
		return data;
	}
};

template<class BlockBufferClass, class FilterClass = FilterDefault>
struct SockBuffer: public BlockBufferClass, public FilterClass {
	using BlockBufferClass::npos;
	using BlockBufferClass::freespace;
	using BlockBufferClass::blocksize;
	using BlockBufferClass::block;
	using BlockBufferClass::max_blocks;
	using BlockBufferClass::tail;
	using BlockBufferClass::size;
	using BlockBufferClass::increase_capacity;
	using BlockBufferClass::append;
	using BlockBufferClass::empty;
	using BlockBufferClass::data;
	using BlockBufferClass::erase;

	using FilterClass::filterWrite;

	typedef FilterClass Filter;
	// 0 : normal close ; >0 : current pump bytes
	int pump(SocketBase& so, size_t n = npos) {
		if (freespace() < (blocksize() >> 1) && block() < max_blocks)
			// ignore increase_capacity result.
			increase_capacity(blocksize());

		size_t nrecv = freespace();
		if (nrecv == 0)
			return -1;
		if (n < nrecv)
			nrecv = n; // min(n, freespace());

		int ret = ::recv(so.m_iSocket, (char*) tail(), nrecv, 0);
		if (ret > 0) {
			filterRead(tail(), ret);
			size(size() + ret);
		}
		return ret;
	}

	////////////////////////////////////////////////////////////////////
	// append into buffer only
	void write(char * msg, size_t size) {
		if (size == 0)
			return;
		if (block() > max_blocks)
			return;

		char *enc = filterWrite(msg, size);
		if (!append(enc, size))
		/* begin change by duzf */
			;
		#if 0
			throw buffer_overflow_all("output buffer overflow [all]");
		#endif
		/* end change by duzf */
	}

	void write(SocketBase& so, SockBuffer & buf) {
		write(so, buf.data(), buf.size());
		buf.erase();
	}

	void write(SocketBase& so, char * msg, size_t size) { // write all / buffered
		if (size == 0)
			return;
		if (block() > max_blocks)
			return;

		char *enc = filterWrite(msg, size);

		size_t nsent = 0;
		if (empty())
			nsent = ::send(so.m_iSocket, (const char*) enc, size, 0);//so.send(enc, size);
		if(nsent == SOCKET_ERROR){
		  nsent = 0;
		}
		if (!append(enc + nsent, size - nsent)) {
			/* begin change by duzf */
			#if 0
			// all or part append error .
			if (nsent > 0)
				throw buffer_overflow("output buffer overflow");
			else
				throw buffer_overflow_all("output buffer overflow [all]");
			#endif
			/* end change by duzf */
		}
	}

	int flush(SocketBase& so, size_t n = npos) {
		size_t nflush = size();
		if (n < nflush)
			nflush = n; // std::min(n, size());
		int ret = ::send(so.m_iSocket, (const char*) data(), nflush, 0);//so.send(data(), nflush);
    if(ret < 0){
      ret = 0;
    }
    erase(0, ret); // if call flush in loop, maybe memmove here
		return ret;
	}
};

#endif // __SNOX_SOCKBUFFER_H_INCLUDE__
