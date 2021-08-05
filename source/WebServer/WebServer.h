//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.5.15
//Description:
#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "base/noncopyable.h"
#include "EventLoop/EventLoop.h"
#include "Thread/ThreadPool.h"
#include "Timer/Timer.h"

#include <map>
#include <atomic>
#include <memory>
#include <functional>
#include <string>

class HttpConnection;
class Socket;
class Channel;
class InetAddress;

class WebServer : noncopyable
{
public:
    WebServer(const InetAddress& listenAddr, int numThreads = 4, int timelimit = 10,
			  int logLevel = 2,  bool saveLog = false, 
			  std::string name = "WebServer");

	~WebServer();

	void start();

private:
	typedef std::map<int, std::shared_ptr<HttpConnection>> ConnectionMap;

	void newConnection();

	void onRead(int fd);
	void onWrite(int fd);
	void onClose(int fd);

	void removeConnection(int fd);
	
	const std::string name_;
	const std::string ipPort_;
	std::atomic<bool> start_;
	int numThreads_;
	EventLoop loop_;
	std::unique_ptr<Socket>  listenSocket_;
	ConnectionMap connections_;
	ThreadPool threadpool_;
	TimerManager timermanager_;
	MutexLock mutex_;
};

#endif //WEBSERVER_H
