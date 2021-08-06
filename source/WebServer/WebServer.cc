#include "WebServer.h"
#include "Socket/InetAddress.h"
#include "Socket/Socket.h"
#include "HttpConn/HttpConnection.h"
#include "Log/Logging.h"
#include "Log/AsyncLogging.h"

#include <assert.h>

namespace 
{
off_t logRollSize = 500*1000*1000;

std::unique_ptr<AsyncLogging> asynclog;

void asyncOutput(const char* msg, int len)
{
  asynclog->append(msg, len);
}
}

WebServer::WebServer(const InetAddress& listenAddr, int numThreads, int timelimit, int logLevel, bool saveLog, std::string name)
	: name_(name),
	  ipPort_(listenAddr.toIpPort()),
	  start_(false),
	  numThreads_(numThreads),
	  listenSocket_(new Socket(listenAddr.family(), Socket::server)),
	  timermanager_(timelimit),
	  mutex_()
{
	Logger::setLogLevel(logLevel);
	if (saveLog) {
		asynclog.reset(new AsyncLogging(name_, "log/", logRollSize));
		asynclog->start();
		Logger::setOutput(asyncOutput);
	}
	threadpool_.setMaxQueueSize(1024);
	listenSocket_->setReuseAddr(true);

	listenSocket_->bindAddress(listenAddr);

	loop_.setListenCallback(std::bind(&WebServer::newConnection, this));
	loop_.setReadCallback(std::bind(&WebServer::onRead, this, std::placeholders::_1));
	loop_.setWriteCallback(std::bind(&WebServer::onWrite, this, std::placeholders::_1));
	loop_.setCloseCallback(std::bind(&WebServer::onClose, this, std::placeholders::_1));
	loop_.setTimerCallback(std::bind(&TimerManager::handleEvent, &timermanager_), 1000);
}

WebServer::~WebServer() 
{
	threadpool_.stop();
	for (auto& item : connections_) {
		HttpConnection::HttpConnectionPtr conn(item.second);
		item.second.reset();
		conn->handleClose();
	}
}

void WebServer::start() {
	assert(!start_);
	start_ = true;
	listenSocket_->listen();
	loop_.enableReading(&*listenSocket_, false);
	threadpool_.start(numThreads_);
	LOG_INFO << "WebServer starts listenning on " << ipPort_;
	loop_.loop();
}

void WebServer::newConnection() {
	int connfd;
	do {
		InetAddress peeraddr;
		connfd = listenSocket_->accept(&peeraddr);
		if (connfd < 0) {
			break;
		}

		HttpConnection::HttpConnectionPtr conn(new HttpConnection(&loop_, connfd));
		conn->setCloseCallback(std::bind(&WebServer::removeConnection, this, std::placeholders::_1));   
		connections_[connfd] = conn;
		timermanager_.addTimer(conn);
		conn->connectEstablished();
		LOG_INFO << "WebServer::newConnection " << "fd = " << connfd <<  " new conn";
	} while(true);
}


void WebServer::onRead(int fd) {
	auto it = connections_.find(fd);
	if (it != connections_.end()) {
		timermanager_.extendTime(it->second);
		threadpool_.run(std::bind(&HttpConnection::handleRead, it->second));
	}
}

void WebServer::onWrite(int fd) {
	auto it = connections_.find(fd);
	if (it != connections_.end()) {
		threadpool_.run(std::bind(&HttpConnection::handleWrite, it->second));
	}
}

void WebServer::onClose(int fd) {  
	auto it = connections_.find(fd);
	if (it != connections_.end()) {
		it->second->handleClose();
	}
}

void WebServer::removeConnection(int fd) {
	// assert(connections_.find(fd) != connections_.end());
	MutexLockGuard lock(mutex_);
	connections_.erase(fd);
}
