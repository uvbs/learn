#ifndef __NonCopyable__H
#define __NonCopyable__H

class Noncopyable
{
protected:
	Noncopyable() {}
	virtual ~Noncopyable() {}
private: 
	Noncopyable(const Noncopyable&);
	const Noncopyable& operator=(const Noncopyable&);
};

#endif //__NonCopyable__H