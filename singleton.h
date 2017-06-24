#ifndef H_SINGLETON
#define H_SINGLETON

#include <cassert>

template<typename T>
class Singleton
{
	static T* singleton;
public:
	Singleton()
	{
		assert(!singleton);
        singleton = static_cast<T*>(this);
	}
	~Singleton()
	{
		assert(singleton);
		singleton = 0;
	}
	static T& GetSingleton()
	{
		assert(singleton);
		return *singleton;
	}
	static T* GetSingletonPtr()
	{
		assert(singleton);
		return singleton;
	}
};

template<typename T> T* Singleton <T>::singleton = 0;

#endif
