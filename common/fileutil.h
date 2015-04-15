#ifndef __FileUtil_H
#define __FileUtil_H

#include <string>
#include <assert.h>

class AutoFp
{
public:
	AutoFp()
	{
		mFp = NULL;
	}

	AutoFp(const wchar_t* filename, const wchar_t* mode)
	{
		mFp = NULL;
		bool rt = open(filename, mode);
		//assert(rt);
	}

	virtual ~AutoFp()
	{
		close();
	}

	void close()
	{
		if(mFp)
		{
			fclose(mFp);
			mFp = NULL;
		}
	}

	bool open(const wchar_t* filename, const wchar_t* mode)
	{
		close();
		mFp = _wfopen(filename, mode);
		if(!mFp)
			return false;
		return true;
	}

	operator FILE* () const
	{
		return get();
	}

	FILE* get() const
	{
		return mFp;
	}

private:
	FILE* mFp;
};


#endif //__FileUtil_H