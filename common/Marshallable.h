#ifndef __Marshellable_H
#define __Marshellable_H

class Marshallable 
{
public:
	virtual ~Marshallable(){}

	virtual int size() const = 0;

	// 返回输出的字节数
	virtual int marshall(void* out, int outSize) const = 0;

	// 成功返回非0,否则返回0
	virtual bool unmarshall(const void* in, int inSize) = 0;
};



#endif // __Marshellable_H