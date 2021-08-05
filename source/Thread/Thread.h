//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.2
//Description:
#ifndef WEBSERVER_THREAD_H
#define WEBSERVER_THREAD_H

#include "base/Atomic.h"
#include "CountDownLatch.h"

#include <functional>
#include <memory>
#include <string>
#include <pthread.h>


class Thread : noncopyable
{
public:
	typedef std::function<void()> ThreadFunc;

	explicit Thread(ThreadFunc, const std::string& name = std::string());
	~Thread();

	void start();
	int join();

	bool started() const { return started_; }
	pid_t tid() const { return tid_; }
	const std::string& name() const { return name_; }

	static int numCreated() { return numCreated_.get(); }

private:
	void setDefaultName();

	bool       started_;
	bool       joined_;
	pthread_t  pthreadId_;
	pid_t      tid_;
	ThreadFunc func_;
	std::string    name_;
	CountDownLatch latch_;

	static AtomicInt32 numCreated_;
};

#endif  // WEBSERVER_THREAD_H
