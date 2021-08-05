//Author: WhiteSheep
//Created: 2021.07.20
//Description: 
//  使用状态机解析用户请求，并保存内容
#ifndef WEBSERVER_HTTPREQUEST_H
#define WEBSERVER_HTTPREQUEST_H

#include <string>
#include <map>
#include <assert.h>
#include <stdio.h>

class Buffer;

class HttpRequest
{
public:
	enum HttpRequestParseState
	{
		kExpectRequestLine,
		kExpectHeaders,
		kExpectBody,
		kGotAll,
	};
	enum Method
	{
		kInvalid, kGet, kPost, kHead, kPut, kDelete
	};
	enum Version
	{
		kUnknown, kHttp10, kHttp11
	};

	HttpRequest()
		: method_(kInvalid),
		version_(kUnknown),
		state_(kExpectRequestLine)
	{ }

	const HttpRequestParseState state() const
	{
		return state_;
	}
	Version getVersion() const
	{
		return version_;
	}
	Method method() const
	{
		return method_;
	}
	const std::string& path() const
	{
		return path_;
	}
	const std::string& query() const
	{
		return query_;
	}
	const std::map<std::string, std::string>& headers() const
	{
		return headers_;
	}
	bool gotAll() const
	{
		return state_ == kGotAll;
	}

	void setVersion(Version v)
	{
		version_ = v;
	}
	void setPath(const char* start, const char* end)
	{
		path_.assign(start, end);
	}
	void setQuery(const char* start, const char* end)
	{
		query_.assign(start, end);
	}
	bool setMethod(const char* start, const char* end)
	{
		assert(method_ == kInvalid);
		std::string m(start, end);
		if (m == "GET")
		{
			method_ = kGet;
		}
		else if (m == "POST")
		{
			method_ = kPost;
		}
		else if (m == "HEAD")
		{
			method_ = kHead;
		}
		else if (m == "PUT")
		{
			method_ = kPut;
		}
		else if (m == "DELETE")
		{
			method_ = kDelete;
		}
		else
		{
			method_ = kInvalid;
		}
		return method_ != kInvalid;
	}
	void addHeader(const char* start, const char* colon, const char* end)
	{
		std::string field(start, colon);
		++colon;
		while (colon < end && isspace(*colon))
		{
			++colon;
		}
		std::string value(colon, end);
		while (!value.empty() && isspace(value[value.size() - 1]))
		{
			value.resize(value.size() - 1);
		}
		headers_[field] = value;
	}


	const char* methodString() const
	{
		const char* result = "UNKNOWN";
		switch (method_)
		{
		case kGet:
			result = "GET";
			break;
		case kPost:
			result = "POST";
			break;
		case kHead:
			result = "HEAD";
			break;
		case kPut:
			result = "PUT";
			break;
		case kDelete:
			result = "DELETE";
			break;
		default:
			break;
		}
		return result;
	}

	std::string getHeader(const std::string& field) const
	{
		std::string result;
		std::map<std::string, std::string>::const_iterator it = headers_.find(field);
		if (it != headers_.end())
		{
			result = it->second;
		}
		return result;
	}

	void swap(HttpRequest& that)
	{
		std::swap(method_, that.method_);
		std::swap(version_, that.version_);
		std::swap(state_, that.state_);
		path_.swap(that.path_);
		query_.swap(that.query_);
		headers_.swap(that.headers_);
	}

	bool parseRequest(Buffer* buf);

private:
	bool processRequestLine(const char* begin, const char* end);

	Method method_;
	Version version_;
	HttpRequestParseState state_;

	std::string path_;
	std::string query_;
	std::map<std::string, std::string> headers_;
};


#endif  // WEBSERVER_HTTPREQUEST_H
