//Author: WhiteSheep
//Created: WhiteSheep  
//Description:
// 		1、负责数据读写
// 		2、http协议解析
// 		3、连接关闭（只重置httpconn的内容）
#ifndef WEBSERVER_HTTTPCONNETCION_H
#define WEBSERVER_HTTTPCONNETCION_H

#include "HttpRequest.h"
#include "base/noncopyable.h"
#include "Buffer/Buffer.h"

#include <memory>
#include <functional>

class Socket;
class EventLoop;
class Entry;

class HttpConnection: noncopyable,
					 public std::enable_shared_from_this<HttpConnection>
{
public:
	friend Entry;    //Timer
	typedef std::shared_ptr<HttpConnection> HttpConnectionPtr;
	typedef std::function<void (int fd)> CloseCallback;
	HttpConnection(EventLoop* loop, int fd);
	
	const int fd() const;

	void connectEstablished();

	void connectionDestroyed();

	void handleRead();
	void handleWrite();
	void handleClose();

	void send(Buffer* buf);
	
	void setCloseCallback(CloseCallback cb) 
	{ closeCallback_ = std::move(cb); }

	void setTimer(const std::weak_ptr<Entry>& weakentry) 
	{ timer_ = weakentry; }

	const std::weak_ptr<Entry>& getTimer() 
	{ return timer_;}

	void reset();

private:
	enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
	void setState(StateE s) { state_ = s; }
	void shutdown();

	EventLoop* loop_;
	HttpRequest request_;
	StateE state_;
	std::unique_ptr<Socket> socket_;
	std::weak_ptr<Entry> timer_;
	bool isClose_;
	CloseCallback closeCallback_;
	Buffer inputBuffer_;
	Buffer outputBuffer_;

};
#endif  //WEBSERVER_HTTTPCONNETCION_H
