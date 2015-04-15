#ifndef __ByteBuffer_H
#define __ByteBuffer_H

#include "common.h"

class ByteBuffer
{
public:
	const static size_t DEFAULT_SIZE = 0x1000;

	ByteBuffer()
		: mReadPos(0)
		, mWritePos(0)
	{
		mStorage.reserve(DEFAULT_SIZE);
	}

	ByteBuffer(size_t res)
		: mReadPos(0)
		, mWritePos(0)
	{
		mStorage.reserve(res);
	}

	ByteBuffer(const ByteBuffer &buf) 
		: mReadPos(buf.mReadPos)
		, mWritePos(buf.mWritePos)
		, mStorage(buf.mStorage)
	{}

	/**///////////////////////////////////////////////////////////////////////////
public:
	void clear()
	{
		mStorage.clear();
		mReadPos = mWritePos = 0;
	}

	template <typename T>
	void append(T value)
	{
		append((uint8_t*)&value, sizeof(value));
	}

	template <typename T>
	void put(size_t pos, T value)
	{
		put(pos, (uint8_t*)&value, sizeof(value));
	}

	/**///////////////////////////////////////////////////////////////////////////
public:
	ByteBuffer& operator<<(bool value)
	{
		append<char>((char)value);
		return *this;
	}
	ByteBuffer& operator<<(uint8_t value)
	{
		append<uint8_t>(value);
		return *this;
	}
	ByteBuffer& operator<<(uint16_t value)
	{
		append<uint16_t>(value);
		return *this;
	}
	ByteBuffer& operator<<(uint32_t value)
	{
		append<uint32_t>(value);
		return *this;
	}
	ByteBuffer& operator<<(uint64_t value)
	{
		append<uint64_t>(value);
		return *this;
	}

	ByteBuffer& operator<<(int8_t value)
	{
		append<int8_t>(value);
		return *this;
	}
	ByteBuffer& operator<<(int16_t value)
	{
		append<int16_t>(value);
		return *this;
	}
	ByteBuffer& operator<<(int32_t value)
	{
		append<int32_t>(value);
		return *this;
	}
	ByteBuffer& operator<<(int64_t value)
	{
		append<int64_t>(value);
		return *this;
	}

	ByteBuffer& operator<<(float value)
	{
		append<float>(value);
		return *this;
	}
	ByteBuffer& operator<<(double value)
	{
		append<double>(value);
		return *this;
	}
	
	/*
	ByteBuffer& operator<<(time_t value)
	{
		append<time_t>(value);
		return *this;
	}
	*/

	ByteBuffer& operator<<(const std::string& value)
	{
		append((uint8_t const *)value.c_str(), value.length());
		append((uint8_t)0);
		return *this;
	}
	ByteBuffer& operator<<(const char* str)
	{
		append( (uint8_t const *)str, str ? strlen(str) : 0);
		append((uint8_t)0);
		return *this;
	}

	/**///////////////////////////////////////////////////////////////////////////
public:
	ByteBuffer& operator>>(bool& value)
	{
		value = read<char>() > 0 ? true : false;
		return *this;
	}
	ByteBuffer& operator>>(uint8_t& value)
	{
		value = read<uint8_t>();
		return *this;
	}
	ByteBuffer& operator>>(uint16_t& value)
	{
		value = read<uint16_t>();
		return *this;
	}
	ByteBuffer& operator>>(uint32_t& value)
	{
		value = read<uint32_t>();
		return *this;
	}
	ByteBuffer& operator>>(uint64_t& value)
	{
		value = read<uint64_t>();
		return *this;
	}

	ByteBuffer& operator>>(int8_t& value)
	{
		value = read<int8_t>();
		return *this;
	}
	ByteBuffer& operator>>(int16_t& value)
	{
		value = read<int16_t>();
		return *this;
	}
	ByteBuffer& operator>>(int32_t& value)
	{
		value = read<int32_t>();
		return *this;
	}
	ByteBuffer& operator>>(int64_t& value)
	{
		value = read<int64_t>();
		return *this;
	}

	ByteBuffer& operator>>(float &value)
	{
		value = read<float>();
		return *this;
	}
	ByteBuffer& operator>>(double &value)
	{
		value = read<double>();
		return *this;
	}
	/*
	ByteBuffer& operator>>(time_t& value)
	{
		value = read<time_t>();
		return *this;
	}
	*/

	ByteBuffer& operator>>(std::string& value)
	{
		value.clear();
		while (rpos() < size())
		{
			char c = read<char>();
			if (c == 0)
			{
				break;
			}
			value += c;
		}
		return *this;
	}

	ByteBuffer& operator>>(char value[])
	{
		std::string strValue;
		strValue.clear();
		while (rpos() < size())
		{
			char c = read<char>();
			if (c == 0)
			{
				break;
			}
			strValue += c;
		}
		strncpy(value, strValue.c_str(), strValue.size());
		return *this;
	}

	/**///////////////////////////////////////////////////////////////////////////
public:
	uint8_t operator[](size_t pos)
	{
		return read<uint8_t>(pos);
	}

	size_t rpos() const
	{
		return mReadPos;
	};

	size_t rpos(size_t rpos_)
	{
		mReadPos = rpos_;
		return mReadPos;
	};

	size_t wpos() const
	{
		return mWritePos;
	}

	size_t wpos(size_t wpos_)
	{
		mWritePos = wpos_;
		return mWritePos;
	}

	template <typename T> T read()
	{
		T r = read<T>(mReadPos);
		mReadPos += sizeof(T);
		return r;
	};
	template <typename T> T read(size_t pos) const
	{
		assert(pos + sizeof(T) <= size() || PrintPosError(false,pos,sizeof(T)));
		return *((T const*)&mStorage[pos]);
	}

	void read(uint8_t *dest, size_t len)
	{
		assert(mReadPos  + len  <= size() || PrintPosError(false, mReadPos,len));
		memcpy(dest, &mStorage[mReadPos], len);
		mReadPos += len;
	}

	const uint8_t* contents() const { return &mStorage[mReadPos]; }

	size_t size() const { return mStorage.size(); }

	bool empty() const { return mStorage.empty(); }

	void resize(size_t _NewSize)
	{
		mStorage.resize(_NewSize);
		mReadPos = 0;
		mWritePos = size();
	};

	void reserve(size_t _Size)
	{
		if (_Size > size()) mStorage.reserve(_Size);
	};

	void append(const std::string& str)
	{
		append((uint8_t const*)str.c_str(), str.size() + 1);
	}
	void append(const char *src, size_t cnt)
	{
		return append((const uint8_t *)src, cnt);
	}
	void append(const uint8_t *src, size_t cnt)
	{
		if (!cnt) return;

		assert(size() < 10000000);

		if (mStorage.size() < mWritePos + cnt)
		{
			mStorage.resize(mWritePos + cnt);
		}
		memcpy(&mStorage[mWritePos], src, cnt);
		mWritePos += cnt;
	}
	void append(const ByteBuffer& buffer)
	{
		if (buffer.size()) append(buffer.contents(),buffer.size());
	}

	void put(size_t pos, const uint8_t *src, size_t cnt)
	{
		assert(pos + cnt <= size() || PrintPosError(true,pos,cnt));
		memcpy(&mStorage[pos], src, cnt);
	}

	void compat()
	{
		const size_t remainderSize = size()-mReadPos;
		if(remainderSize > 0)
			memmove(&mStorage[0], &mStorage[mReadPos], remainderSize);
		resize(remainderSize);
		wpos(remainderSize);
		rpos(0);
	}

	/**///////////////////////////////////////////////////////////////////////////
public:
	void print_storage()
	{
	}

	void textlike()
	{
	}

	void hexlike()
	{
	}

	bool PrintPosError(bool add, size_t pos, size_t esize) const
	{
		printf("ERROR: Attempt %s in ByteBuffer (pos: %u size: %u) value with size: %u",(add ? "put" : "get"), pos, size(), esize);
		return false;
	}

protected:
	size_t                mReadPos;
	size_t                mWritePos;
	std::vector<uint8_t>    mStorage;
};


/**///////////////////////////////////////////////////////////////////////////
// std::vector
/**///////////////////////////////////////////////////////////////////////////
#ifdef _VECTOR_
template <typename T>
ByteBuffer& operator<<(ByteBuffer& b, const std::vector<T>& v)
{
	b << (uint32_t)v.size();

	typename std::vector<T>::const_iterator iter    = v.begin();
	typename std::vector<T>::const_iterator& iEnd    = v.end();
	for (; iter != iEnd; ++iter)
	{
		b << *iter;
	}
	return b;
}

template <typename T>
ByteBuffer& operator>>(ByteBuffer& b, std::vector<T>& v)
{
	uint32_t vsize;
	b >> vsize;
	v.clear();
	while (vsize--)
	{
		T t;
		b >> t;
		v.push_back(t);
	}
	return b;
}
#endif

/**///////////////////////////////////////////////////////////////////////////
// std::list
/**///////////////////////////////////////////////////////////////////////////
#ifdef _LIST_
template <typename T>
ByteBuffer& operator<<(ByteBuffer& b, const std::list<T>& v)
{
	b << (uint32_t)v.size();

	typename std::list<T>::const_iterator iter    = v.begin();
	typename std::list<T>::const_iterator& iEnd    = v.end();
	for (; iter != iEnd; ++iter)
	{
		b << *iter;
	}
	return b;
}

template <typename T>
ByteBuffer& operator>>(ByteBuffer& b, std::list<T>& v)
{
	uint32_t vsize;
	b >> vsize;
	v.clear();
	while (vsize--)
	{
		T t;
		b >> t;
		v.push_back(t);
	}
	return b;
}
#endif

/**///////////////////////////////////////////////////////////////////////////
// std::map
/**///////////////////////////////////////////////////////////////////////////
#ifdef _MAP_
template <typename K, typename V>
ByteBuffer& operator<<(ByteBuffer& b, const std::map<K, V>& m)
{
	b << (uint32_t)m.size();

	typename std::map<K, V>::const_iterator iter = m.begin();
	typename std::map<K, V>::const_iterator iEnd = m.end();
	for (; iter != iEnd; ++iter)
	{
		b << iter->first << iter->second;
	}
	return b;
}

template <typename K, typename V>
ByteBuffer &operator>>(ByteBuffer& b, std::map<K, V>& m)
{
	uint32_t msize;
	b >> msize;
	m.clear();
	while (msize--)
	{
		K k;
		V v;
		b >> k >> v;
		m.insert(std::make_pair(k, v));
	}
	return b;
}

#endif 
#endif // __ByteBuffer_H
