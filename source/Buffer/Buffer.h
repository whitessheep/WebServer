//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.2
//Description:
#ifndef WEBSERVER_BUFFER_H
#define WEBSERVER_BUFFER_H

#include "base/StringPiece.h"

#include <algorithm>
#include <string>
#include <vector>
#include <assert.h>
#include <string.h>

class Buffer
{
public:
	static const size_t kInitialSize = 1024;
	explicit Buffer(size_t initialSize = kInitialSize)
		: buffer_(initialSize),
		readerIndex_(0),
		writerIndex_(0)
	{
		assert(readableBytes() == 0);
		assert(writableBytes() == initialSize);
	}

	void swap(Buffer& rhs) {
		buffer_.swap(rhs.buffer_);
		std::swap(readerIndex_, rhs.readerIndex_);
		std::swap(readerIndex_, rhs.readerIndex_);
	}

	size_t readableBytes() const {
		return writerIndex_ - readerIndex_;
	}

	size_t writableBytes() const {
		return buffer_.size() - writerIndex_;
	}

	size_t internalCapacity() const {
		return buffer_.capacity();
	}


	const char* peek() const {
		return begin() + readerIndex_;
	}

	void retrieve(size_t len) {
		assert(len <= readableBytes());
		if (len < readableBytes()) {
			readerIndex_ += len;
		}
		else {
			retrieveAll();
		}
	}

	void retrieveUntil(const char* end) {
		assert(peek() <= end);
		assert(end <= beginWrite());
		retrieve(end - peek());
	}

	void retrieveAll() {
		readerIndex_ = 0;
		writerIndex_ = 0;
	}

	std::string retrieveAsString(size_t len) {
		std::string str(peek(), len);
		retrieve(len);
		return str;
	}

	std::string retrieveAllAsString() {
		return retrieveAsString(readableBytes());
	}

	StringPiece toStringPiece() const {
		return StringPiece(peek(), static_cast<int>(readableBytes()));
	}

	void append(const StringPiece& str) {
		append(str.data(), str.size());
	}

	void append(const char* data, size_t len) {
		ensureWriteBytes(len);
		std::copy(data, data + len, beginWrite());
		hasWritten(len);
	}

	void append(const void* data, size_t len) {
		append(static_cast<const char*> (data), len);
	}

	void ensureWriteBytes(size_t len) {
		if (writableBytes() < len) {
			makeSpace(len);
		}
		assert(writableBytes() >= len);
	}

	char* beginWrite() {
		return begin() + writerIndex_;
	}

	const char* beginWrite() const {
		return begin() + writerIndex_;
	}

	void hasWritten(size_t len) {
		assert(len <= writableBytes());
		writerIndex_ += len;
	}

	const char* findCRLF() const {
		const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
		return crlf == beginWrite() ? NULL : crlf;
	}

	const char* findCRLF(const char* start) const {
		const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
		return crlf == beginWrite() ? NULL : crlf;
	}
	const char* findEOL() const {
		const void* eol = memchr(peek(), '\n', readableBytes());
		return static_cast<const char*>(eol);
	}

	const char* findEOL(const char* start) const
	{
		assert(peek() <= start);
		assert(start <= beginWrite());
		const void* eol = memchr(start, '\n', beginWrite() - start);
		return static_cast<const char*>(eol);
	}

	ssize_t readFd(int fd, int* savedErrno);

private:
	char* begin() {
		return &*buffer_.begin();
	}

	const char* begin() const {
		return &*buffer_.begin();
	}

	void makeSpace(size_t len) {
		if (writableBytes() + readableBytes() < len) {
			buffer_.resize(writerIndex_ + len);
		}
		else {
			size_t readable = readableBytes();
			std::copy(begin() + readerIndex_, begin() + writerIndex_, begin());
			readerIndex_ = 0;
			writerIndex_ = readerIndex_ + readable;
			assert(readable == readableBytes());
		}
	}

private:
	std::vector<char> buffer_;
	size_t readerIndex_;
	size_t writerIndex_;

	static const char kCRLF[];
};


#endif //WEBSERVER_BUFFER_H 
