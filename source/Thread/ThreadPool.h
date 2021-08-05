//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.2
//Description:
#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H

#include "base/Condition.h"
#include "base/Mutex.h"
#include "Thread.h"

#include <deque>
#include <vector>
#include <string>


class ThreadPool : noncopyable
{
public:
	typedef std::function<void()> Task;

	explicit ThreadPool(const std::string& nameArg = std::string("ThreadPool"));
	~ThreadPool();

	void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
	void setThreadInitCallback(const Task& cb)
	{
		threadInitCallback_ = cb;
	}

	void start(int numThreads);
	void stop();

	const std::string& name() const
	{
		return name_;
	}

	size_t queueSize() const;

	void run(Task f);

private:
	bool isFull() const;
	void runInThread();
	Task take();

	mutable MutexLock mutex_;
	Condition notEmpty_;
	Condition notFull_;
	std::string name_;
	Task threadInitCallback_;
	std::vector<std::unique_ptr<Thread>> threads_;
	std::deque<Task> queue_;
	size_t maxQueueSize_;
	bool running_;
};



#endif //WEBSERVER_THREADPOOL_H
