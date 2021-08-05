//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.2
//Description:
#ifndef WEBSERVER_SOCKET_H
#define WEBSERVER_SOCKET_H

#include "base/noncopyable.h"
#include "arpa/inet.h"

class InetAddress;

class Socket : noncopyable
{
public:
	enum SocketType
	{
		server,
		client,
	};

	Socket(sa_family_t family, SocketType type);
	Socket(int sockfd, SocketType type) : fd_(sockfd), events_(0), type_(type) { }
	~Socket();


	void bindAddress(const InetAddress& localaddr);
	void listen();

	int accept(InetAddress* peeraddr);

	const int fd() const { return fd_; }

	void shutdownWrite();

	void setTcpNoDelay(bool on);
	void setReuseAddr(bool on);
	void setReusePort(bool on);
	void setKeepAlive(bool on);


	void setEvents(int events) { events_ = events; }
	void modEvents(int events) { events_ |= events; }
	void removeEvents(int events) { events_ &= events; }

	const int events() const { return events_; }
	const SocketType type() const { return type_; }

private:
	int createNonBlockAndCloseOnExecfd(sa_family_t family);
	void setNonBlockAndCloseOnExec(int sockfd);


	int fd_;
	int events_;
	SocketType type_;
	static int idleFd_;
};

#endif //WEBSERVER_SOCKET_H
