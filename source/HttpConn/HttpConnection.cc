#include "HttpConnection.h"
#include "EventLoop/EventLoop.h"
#include "Timer/Timer.h"
#include "HttpResponse.h"
#include "Socket/Socket.h"
#include "Log/Logging.h"

#include <unistd.h>

HttpConnection::HttpConnection(EventLoop* loop, int fd)
	: loop_(loop),
	state_(kConnecting),
	socket_(new Socket(fd, Socket::client)),
	isClose_(false)
{
	socket_->setKeepAlive(true);
	// socket_->setTcpNoDelay(true);
}


void HttpConnection::connectEstablished() {
	assert(state_ == kConnecting);
	setState(kConnected);
	loop_->enableReading(&*socket_);
}

const int HttpConnection::fd() const {
	return socket_->fd();
}

void HttpConnection::reset() {
	HttpRequest dummy;
	request_.swap(dummy);
}

void HttpConnection::handleRead() {
	int saveErrno = 0;
	ssize_t n;
	do {
		n = inputBuffer_.readFd(socket_->fd(), &saveErrno);
		if (n <= 0) {
			if (n == 0) {
				handleClose();
				return;
			}
			else if (saveErrno == EAGAIN) {
				break;
			}
			else {
				LOG_WARN << "HttpConnection::handleRead unknown error fd = " << socket_->fd();
				handleClose();
				return;
			}
		}
	} while (true);

	if (request_.parseRequest(&inputBuffer_)) {
		if (request_.gotAll()) {
			LOG_TRACE << "HttpConnection::handleRead fd = " << socket_->fd() << " requset got all";
			const string& connection = request_.getHeader("Connection");
			bool close = connection == "close" ||
				(request_.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
			HttpResponse response(close);
			response.onResponse(request_);
			response.appendToBuffer(&outputBuffer_);
			send(&outputBuffer_);
			if (response.closeConnection()) {
				shutdown();
			}
			reset();
		}
		else {
			loop_->resetReading(&*socket_);
		}
	}
	else {
		LOG_TRACE << "HttpConnection::handleRead fd = " << socket_->fd() << " bad request";
		Buffer buffer;
		buffer.append("HTTP/1.1 400 Bad Request\r\n\r\n");
		send(&buffer);
		shutdown();
	}
}

void HttpConnection::handleWrite() {
	if (loop_->checkWriting(*socket_)) {
		ssize_t n;
		do {
			n = write(socket_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
			if (n > 0) {
				outputBuffer_.retrieve(n);
			}
		} while (n > 0);
		if (outputBuffer_.readableBytes() == 0) {
			loop_->resetReading(&*socket_);
			if (state_ == kDisconnecting) {
				shutdown();
			}
		}
		else if (errno == EAGAIN) {
			loop_->resetWriting(&*socket_);
		}
		else {
			LOG_WARN << "HttpConnection::handleWrite unknown error fd = " << socket_->fd();
			handleClose();
		}
	}
}

void HttpConnection::handleClose() {
	LOG_INFO << "HttpConnection::handleClose fd = " << socket_->fd();
	loop_->removeEvents(&*socket_);
	setState(kDisconnected);
	closeCallback_(socket_->fd());
}

void HttpConnection::shutdown() {
	if (state_ == kConnected) {
		setState(kDisconnecting);
	}
	if (!loop_->checkWriting(*socket_)) {
		socket_->shutdownWrite();
	}
}

void HttpConnection::connectionDestroyed() {
	LOG_INFO << "HttpConnection::connectionDestroyed fd = " << socket_->fd();
	loop_->removeEvents(&*socket_);
	setState(kDisconnected);
}

void HttpConnection::send(Buffer* buf) {
	if (state_ != kConnected) {
		loop_->resetReading(&*socket_);
		return;
	}
	ssize_t nwrote = 0;
	size_t remaining = buf->readableBytes();
	bool faultError = false;
	if (!loop_->checkWriting(*socket_)) {
		nwrote = write(socket_->fd(), buf->peek(), remaining);
		if (nwrote >= 0) {
			buf->retrieve(nwrote);
			remaining -= nwrote;
		}
		else {
			nwrote = 0;
			if (errno != EWOULDBLOCK) {
				if (errno == EPIPE || errno == ECONNRESET) {
					faultError = true;
				}
			}
		}
		loop_->resetReading(&*socket_);
	}

	if (!faultError && remaining > 0) {
		if (!loop_->checkWriting(*socket_)) {
			loop_->resetWriting(&*socket_);
		}
	}
}
