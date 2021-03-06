//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.2
//Description:
#ifndef WEBSERVER_FILEUTIL_H
#define WEBSERVER_FILEUTIL_H

#include "base/noncopyable.h"
#include <sys/types.h>  // for off_t
#include <string>

class AppendFile : noncopyable
{
public:
	explicit AppendFile(const std::string& filename);

	~AppendFile();

	void append(const char* logline, size_t len);

	void flush();

	off_t writtenBytes() const { return writtenBytes_; }

private:

	size_t write(const char* logline, size_t len);

	FILE* fp_;
	char buffer_[64 * 1024];
	off_t writtenBytes_;
};


#endif //WEBSERVER_FILEUTIL_H

