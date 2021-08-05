#include "HttpRequest.h"
#include "Buffer/Buffer.h"

//只判断并只判断一次头部解析正确与否
bool HttpRequest::parseRequest(Buffer* buf)
{
	bool ok = true;
	bool hasMore = true;
	while (hasMore)
	{
		//解析请求行
		if (state_ == kExpectRequestLine)
		{
			const char* crlf = buf->findCRLF();
			if (crlf)
			{
				ok = processRequestLine(buf->peek(), crlf);
				//状态转移，同时释放buf
				if (ok)
				{
					buf->retrieveUntil(crlf + 2);
					state_ = kExpectHeaders;
				}
				else
				{
					hasMore = false;
				}
			}
			else
			{
				hasMore = false;
			}
		}
		//解析头部
		else if (state_ == kExpectHeaders)
		{
			const char* crlf = buf->findCRLF();
			if (crlf)
			{
				const char* colon = std::find(buf->peek(), crlf, ':');
				if (colon != crlf)
				{
					addHeader(buf->peek(), colon, crlf);
				}
				else
				{
					state_ = kGotAll;
					hasMore = false;
				}
				buf->retrieveUntil(crlf + 2);
			}
			else
			{
				hasMore = false;
			}
		}
		else if (state_ == kExpectBody)
		{
			// FIXME:   清空body
		}
	}
	return ok;
}

bool HttpRequest::processRequestLine(const char* begin, const char* end)
{
	bool succeed = false;
	const char* start = begin;
	const char* space = std::find(start, end, ' ');
	if (space != end && setMethod(start, space))
	{
		start = space + 1;
		space = std::find(start, end, ' ');
		if (space != end)
		{
			const char* question = std::find(start, space, '?');
			if (question != space)
			{
				setPath(start, question);
				setQuery(question, space);
			}
			else
			{
				setPath(start, space);
			}
			start = space + 1;
			succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
			if (succeed)
			{
				if (*(end - 1) == '1')
				{
					setVersion(HttpRequest::kHttp11);
				}
				else if (*(end - 1) == '0')
				{
					setVersion(HttpRequest::kHttp10);
				}
				else
				{
					succeed = false;
				}
			}
		}
	}
	return succeed;
}

