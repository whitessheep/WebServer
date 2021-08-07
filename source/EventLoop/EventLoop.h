//Author: WhiteSheep
//Created: 2021.7.25
//Description:
//
#ifndef WEBSERVER_EVENTLOOP_H
#define WEBSERVER_EVENTLOOP_H

#include "base/noncopyable.h"
#include "base/Mutex.h"

#include <atomic>
#include <vector>
#include <functional>
class Socket;

class EventLoop: noncopyable 
{
public:
	typedef std::function<void ()> EventCallback; 
	typedef std::function<void (int fd)> ConnectionCallback;
	EventLoop();
	~EventLoop();

	void loop();

	void setListenCallback(EventCallback cb)
	{ listenCallback_ = std::move(cb); }
    void setReadCallback(ConnectionCallback cb)
    { readCallback_ = std::move(cb); }
    void setWriteCallback(ConnectionCallback cb)
    { writeCallback_ = std::move(cb); }
    void setCloseCallback(ConnectionCallback cb)
    { closeCallback_ = std::move(cb); }
    void setErrorCallback(ConnectionCallback cb)
    { errorCallback_ = std::move(cb); }    
	void setTimerCallback(EventCallback cb, int timeout)
	{ TimerCallback_ = std::move(cb); timeout_ = timeout; }

	void enableReading(Socket* socket, bool oneshot = true);
	void resetWriting(Socket* socket);
	void resetReading(Socket* socket);
	void removeEvents(Socket* socket);
	bool checkWriting(const Socket& socket);

	void queueInLoop(EventCallback cb);
private:
	static const int kInitEventListSize = 1024;
	typedef std::vector<struct epoll_event> EventList;

    static const int kNoneEvent;    
    static const int kDefaultEvent; 
    static const int kReadEvent;    
    static const int kWriteEvent;  

	int  waitEvents();
	void handleEvent(int numEvents);
	void doPendingFunctors();	

	bool looping_;
	std::atomic<bool> quit_;
	int epollfd_;
	EventList events_;
	int timeout_;

	EventCallback listenCallback_;
	ConnectionCallback readCallback_;
	ConnectionCallback writeCallback_;
	ConnectionCallback closeCallback_;
	ConnectionCallback errorCallback_;
	EventCallback TimerCallback_;

	MutexLock mutex_;
	std::vector<EventCallback> pendingFunctors_ ;
};

#endif //WEBSERVER_EVENTLOOP_H
