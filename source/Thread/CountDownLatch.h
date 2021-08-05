//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.2
//Description:
#ifndef WEBSERVER_COUNTDOWNLATCH_H
#define WEBSERVER_COUNTDOWNLATCH_H

#include "base/Condition.h"
#include "base/Mutex.h"


class CountDownLatch : noncopyable
{
public:

	explicit CountDownLatch(int count);

	void wait();

	void countDown();

	int getCount() const;

private:
	mutable MutexLock mutex_;
	Condition condition_;
	int count_;
};

#endif  // WEBSERVER_COUNTDOWNLATCH_H
