#include "HttpResponse.h"
#include "HttpRequest.h"
#include "Buffer/Buffer.h"
#include "Log/Logging.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

namespace
{
	char INDEX[] = "<!DOCTYPE html>\n"
		"<html>\n"
		"<head>\n"
		"    <title>Welcome to WebServer!</title>\n"
		"    <style>\n"
		"        body {\n"
		"            width: 35em;\n"
		"            margin: 0 auto;\n"
		"            font-family: Tahoma, Verdana, Arial, sans-serif;\n"
		"        }\n"
		"    </style>\n"
		"</head>\n"
		"<body>\n"
		"<h1>Welcome to WebServer!</h1>\n"
		"</body>\n"
		"</html>";

	const string root = ".";
}

void HttpResponse::onResponse(const HttpRequest& req)
{
	LOG_DEBUG << "Headers " << req.methodString() << " " << req.path();
	bool benchmark = true;
	if (!benchmark)
	{
		const std::map<string, string>& headers = req.headers();
		for (const auto& header : headers)
		{
			LOG_TRACE << header.first << ": " << header.second;
		}
	}

	if (req.path() == "/")
	{
		setStatusCode(HttpResponse::k200Ok);
		setStatusMessage("OK");
		setContentType("text/html");
		addHeader("Server", "WhiteSheep");
		setBody(INDEX);
	}
	else if (req.path() == "/hello")
	{
		setStatusCode(HttpResponse::k200Ok);
		setStatusMessage("OK");
		setContentType("text/plain");
		addHeader("Server", "WhiteSheep");
		setBody("hello, world!\n");
	}
	else    //file
	{
		struct stat fileStat;
		string path = root + req.path();
		if (stat(path.c_str(), &fileStat) < 0) {
			setStatusCode(HttpResponse::k404NotFound);
			setStatusMessage("Not Found");
			setCloseConnection(true);
			return;
		}
		int fd = open(path.c_str(), O_RDONLY);
		if (fd < 0) {
			setStatusCode(HttpResponse::k404NotFound);
			setStatusMessage("Not Found");
			setCloseConnection(true);
			return;
		}
		void* mapbuf = mmap(NULL, fileStat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
		close(fd);
		if (mapbuf == (void*)-1) {
			setStatusCode(HttpResponse::k404NotFound);
			setStatusMessage("Not Found");
			setCloseConnection(true);
			return;
		}
		setStatusMessage("OK");
		setContentType("text/plain");
		addHeader("Server", "WhiteSheep");
		setBody(static_cast<char*>(mapbuf));
	}
}

void HttpResponse::appendToBuffer(Buffer* output) const
{
	char buf[32];
	snprintf(buf, sizeof buf, "HTTP/1.1 %d ", statusCode_);
	output->append(buf);
	output->append(statusMessage_);
	output->append("\r\n");

	if (closeConnection_)
	{
		output->append("Connection: close\r\n");
	}
	else
	{
		snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", body_.size());
		output->append(buf);
		output->append("Connection: Keep-Alive\r\n");
	}

	for (const auto& header : headers_)
	{
		output->append(header.first);
		output->append(": ");
		output->append(header.second);
		output->append("\r\n");
	}

	output->append("\r\n");
	output->append(body_);
}
