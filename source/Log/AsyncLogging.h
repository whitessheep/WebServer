//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.2
//Description:
#ifndef WEBSERVER_ASYNCLOGGING_H
#define WEBSERVER_ASYNCLOGGING_H

#include "Thread/CountDownLatch.h"
#include "base/Mutex.h"
#include "Thread/Thread.h"
#include "LogStream.h"

#include <atomic>
#include <vector>


class AsyncLogging : noncopyable
{
public:
	AsyncLogging(const string& basename,
		const string& path,
		off_t rollSize,
		int flushInterval = 3);

	~AsyncLogging()
	{
		if (running_)
		{
			stop();
		}
	}

	void append(const char* logline, int len);

	void start()
	{
		running_ = true;
		thread_.start();
		latch_.wait();
	}

	void stop()
	{
		running_ = false;
		cond_.notify();
		thread_.join();
	}

private:
	void threadFunc();

	typedef FixedBuffer<kLargeBuffer> Buffer;
	typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
	typedef BufferVector::value_type BufferPtr;

	const int flushInterval_;
	std::atomic<bool> running_;
	const string basename_;
	const string path_;
	const off_t rollSize_;
	Thread thread_;
	CountDownLatch latch_;
	MutexLock mutex_;
	Condition cond_;
	BufferPtr currentBuffer_;
	BufferPtr nextBuffer_;
	BufferVector buffers_;
};

#endif  // WEBSERVER_ASYNCLOGGING_H
