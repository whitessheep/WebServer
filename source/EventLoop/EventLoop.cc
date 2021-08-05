#include "EventLoop.h"
#include "Socket/Socket.h"
#include "Log/Logging.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>

const int EventLoop::kNoneEvent = 0;
const int EventLoop::kDefaultEvent = EPOLLONESHOT | EPOLLET | EPOLLRDHUP;
const int EventLoop::kReadEvent = EPOLLIN | EPOLLPRI;
const int EventLoop::kWriteEvent = EPOLLOUT;

namespace
{
	class IgnoreSigPipe
	{
	public:
		IgnoreSigPipe()
		{
			::signal(SIGPIPE, SIG_IGN);
		}
	};

	IgnoreSigPipe initObj;
}  // namespace

EventLoop::EventLoop()
	: looping_(false),
	quit_(false),
	epollfd_(epoll_create1(EPOLL_CLOEXEC)),
	events_(kInitEventListSize),
	timeout_(-1)
{
	assert(epollfd_ >= 0);
}

EventLoop::~EventLoop()
{
	close(epollfd_);
}

void EventLoop::loop() {
	assert(!looping_);
	looping_ = true;

	while (!quit_) {
		// events_.clear();
		int numEvents = waitEvents();
		handleEvent(numEvents);
	}
}

int EventLoop::waitEvents() {
	int numEvents = epoll_wait(epollfd_,
		&*events_.begin(),
		static_cast<int>(events_.size()),
		timeout_);
	int savedErrno = errno;
	if (numEvents > 0) {
		LOG_TRACE << "EventLoop::waitEvents events happened";
		if (static_cast<size_t>(numEvents) == events_.size()) {
			events_.resize(events_.size() * 2);
		}
	}
	else if (numEvents == 0) {
		LOG_TRACE << "EventLoop::waitEvents nothing events";
	}
	else {
		if (savedErrno != EINTR) {
			errno = savedErrno;
			LOG_TRACE << "EventLoop::waitEvents unknown error";
		}
	}
	return numEvents;
}

void EventLoop::handleEvent(int numEvents) {
	for (int i = 0; i < numEvents; ++i) {
		auto it = static_cast<Socket*>(events_[i].data.ptr);
		if (it->type() == Socket::server) {
			if (events_[i].events & EPOLLIN) {
				if (listenCallback_) {
					listenCallback_();
				}
			}
		}
		else {
			int fd = it->fd();
			if ((events_[i].events & EPOLLHUP) && !(events_[i].events & EPOLLIN)) {
				if (closeCallback_) closeCallback_(fd);
			}
			if (events_[i].events & EPOLLRDHUP) {
				if (closeCallback_) closeCallback_(fd);
			}

			if (events_[i].events & EPOLLERR) {
				if (errorCallback_) errorCallback_(fd);
			}

			if (events_[i].events & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
				if (readCallback_) readCallback_(fd);
			}

			if (events_[i].events & EPOLLOUT) {
				if (writeCallback_) writeCallback_(fd);
			}
		}
	}
	if (TimerCallback_) {
		TimerCallback_();
	}
}

void EventLoop::enableReading(Socket* socket, bool oneshot) {
	if (!oneshot) {
		socket->setEvents(EPOLLET | kReadEvent);
	}
	else {
		socket->setEvents(kDefaultEvent | kReadEvent);
	}
	struct epoll_event event;
	memset(&event, 0, sizeof event);
	event.data.ptr = socket;
	event.events = socket->events();
	epoll_ctl(epollfd_, EPOLL_CTL_ADD, socket->fd(), &event);
}

void EventLoop::resetReading(Socket* socket) {
	socket->setEvents(kDefaultEvent | kReadEvent);
	struct epoll_event event;
	memset(&event, 0, sizeof event);
	event.data.ptr = socket;
	event.events = socket->events();
	epoll_ctl(epollfd_, EPOLL_CTL_MOD, socket->fd(), &event);
}

void EventLoop::resetWriting(Socket* socket) {
	socket->modEvents(kWriteEvent);
	struct epoll_event event;
	memset(&event, 0, sizeof event);
	event.data.ptr = socket;
	event.events = socket->events();
	epoll_ctl(epollfd_, EPOLL_CTL_MOD, socket->fd(), &event);
}

void EventLoop::removeEvents(Socket* socket) {
	socket->setEvents(kNoneEvent);
	struct epoll_event event;
	memset(&event, 0, sizeof event);
	event.data.ptr = socket;
	event.events = socket->events();
	epoll_ctl(epollfd_, EPOLL_CTL_DEL, socket->fd(), &event);

}

bool EventLoop::checkWriting(const Socket& socket) {
	return socket.events() & kWriteEvent;
}


