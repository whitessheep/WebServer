//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.2
//Description:
#ifndef WEBSERVER_CURRENTTHREAD_H
#define WEBSERVER_CURRENTTHREAD_H


namespace CurrentThread
{
	extern __thread int t_cachedTid;
	extern __thread char t_tidString[32];
	extern __thread int t_tidStringLength;
	extern __thread const char* t_threadName;
	void cacheTid();

	inline int tid()
	{
		if (__builtin_expect(t_cachedTid == 0, 0))
		{
			cacheTid();
		}
		return t_cachedTid;
	}

	inline const char* tidString() // for logging
	{
		return t_tidString;
	}

	inline int tidStringLength() // for logging
	{
		return t_tidStringLength;
	}

	inline const char* name()
	{
		return t_threadName;
	}

	bool isMainThread();


}  // namespace CurrentThread

#endif //WEBSERVER_CURRENTTHREAD_H
