#include "Socket.h"
#include "InetAddress.h"
#include "Log/Logging.h"

#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h> 
#include <netinet/tcp.h>
#include <unistd.h>
#include <errno.h>

int Socket::idleFd_ = open("/dev/null", O_RDONLY | O_CLOEXEC);

Socket::Socket(sa_family_t family, SocketType type)
	: fd_(createNonBlockAndCloseOnExecfd(family)),
	events_(0),
	type_(type)
{ }

Socket::~Socket() {
	::close(fd_);
}


void Socket::bindAddress(const InetAddress& localhost) {
	int ret = ::bind(fd_, localhost.getSockAddr(), static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
	if (ret < 0) {
		LOG_FATAL << "sockets::bindAddress failed ";
	}
}

void Socket::listen() {
	int ret = ::listen(fd_, SOMAXCONN);
	if (ret < 0) {
		LOG_FATAL << "sockets::listen";
	}
}

int Socket::accept(InetAddress* peeraddr) {
	sockaddr_in6 addr;
	memset(&addr, 0, sizeof addr);
	socklen_t addrlen = static_cast<socklen_t>(sizeof addr);
#if VALGRIND || defined (NO_ACCEPT4)
	int connfd = ::accept(fd_, (sockaddr*)&addr, &addrlen);
	setNonBlockAndCloseOnExec(connfd);
#else
	int connfd = ::accept4(fd_, (sockaddr*)&addr,
		&addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
	if (connfd < 0 && errno != EAGAIN)
	{
		int savedErrno = errno;
		LOG_ERROR << "Socket::accept";
		switch (savedErrno)
		{
		case EMFILE:
			::close(idleFd_);
      		idleFd_ = ::accept(fd_, NULL, NULL);
      		::close(idleFd_);
      		idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
		case ECONNABORTED:
		case EINTR:
		case EPROTO:
		case EPERM:
			errno = savedErrno;
			break;
		case EBADF:
		case EFAULT:
		case EINVAL:
		case ENFILE:
		case ENOBUFS:
		case ENOMEM:
		case ENOTSOCK:
		case EOPNOTSUPP:
			LOG_FATAL << "unexpected error of ::accept " << savedErrno;
			break;
		default:
			LOG_FATAL << "unknown error of ::accept " << savedErrno;
			break;
		}
	}
	else {
		peeraddr->setSockAddrInet6(addr);
	}
	return connfd;
}

int Socket::createNonBlockAndCloseOnExecfd(sa_family_t family)
{
#if VALGRIND
	int sockfd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd < 0)
	{
		LOG_FATAL << "sockets::createNonBlockAndCloseOnExecfd";
	}

	setNonBlockAndCloseOnExec(sockfd);
#else
	int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
	if (sockfd < 0)
	{
		LOG_FATAL << "sockets::createNonBlockAndCloseOnExecfd";
	}
#endif
	return sockfd;
}

void Socket::shutdownWrite()
{
	if(shutdown(fd_, SHUT_WR) < 0) {
		LOG_ERROR << "sockets::shutdownWrite";
	}
}

void Socket::setTcpNoDelay(bool on)
{
	int optval = on ? 1 : 0;
	int ret = setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY,
		&optval, static_cast<socklen_t>(sizeof optval));
	if (ret < 0)
	{
		LOG_ERROR << "sockets::setTcpNoDelay failed fd = " << fd_;
	}
}

void Socket::setReuseAddr(bool on)
{
	int optval = on ? 1 : 0;
	int ret = setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR,
		&optval, static_cast<socklen_t>(sizeof optval));
	if (ret < 0) {
		LOG_FATAL << "sockets::setReuseAddr failed fd = " << fd_;
	}
}

void Socket::setReusePort(bool on)
{
#ifdef SO_REUSEPORT
	int optval = on ? 1 : 0;
	int ret = setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT,
		&optval, static_cast<socklen_t>(sizeof optval));
	if (ret < 0 && on)
	{
		LOG_ERROR << "SO_REUSEPORT failed fd = " << fd_;
	}
#else
	if (on)
	{
		LOG_ERROR << "SO_REUSEPORT is not supported fd = " << fd_;
	}
#endif
}

void Socket::setKeepAlive(bool on)
{
	int optval = on ? 1 : 0;
	int ret = setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE,
		&optval, static_cast<socklen_t>(sizeof optval));
	if (ret < 0) {
		LOG_ERROR << "sockets::setKeepAlive failed fd = " << fd_;
	}
}

