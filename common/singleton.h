#ifndef __Singleton__H
#define __Singleton__H

#include <assert.h>

template <typename T> 
class Singleton
{
protected:
	static T* mInstance;
	Singleton(const Singleton& o) { assert(0); }
public:
	Singleton( void ) { assert( !mInstance ); mInstance = static_cast<T*>(this); }
	~Singleton( void ) {  assert( mInstance );  mInstance = 0; }
	static T& getInstance( void ) {  assert( mInstance );  return ( *mInstance );  }
	static T* getInstancePtr( void ) {  return ( mInstance );  }
};
template <typename T> T* Singleton<T>::mInstance = NULL;

#endif // __Singleton__H