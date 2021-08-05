#include "Logging.h"
#include "Thread/CurrentThread.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sstream>
#include <sys/time.h>

__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_lastSecond;

const char* strerror_tl(int savedErrno)
{
	return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
}

Logger::LogLevel initLogLevel()
{
	if (::getenv("LOG_TRACE"))
		return Logger::TRACE;
	else if (::getenv("LOG_DEBUG"))
		return Logger::DEBUG;
	else
		return Logger::INFO;
}

Logger::LogLevel g_logLevel = initLogLevel();

const char* LogLevelName[Logger::NONE] =
{
  "TRACE ",
  "DEBUG ",
  "INFO  ",
  "WARN  ",
  "ERROR ",
  "FATAL ",
};

class T
{
public:
	T(const char* str, unsigned len)
		:str_(str),
		len_(len)
	{
		assert(strlen(str) == len_);
	}

	const char* str_;
	const unsigned len_;
};

inline LogStream& operator<<(LogStream& s, T v)
{
	s.append(v.str_, v.len_);
	return s;
}

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v)
{
	s.append(v.data_, v.size_);
	return s;
}

void defaultOutput(const char* msg, int len)
{
	size_t n = fwrite(msg, 1, len, stdout);
	(void)n;
}

void defaultFlush()
{
	fflush(stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;


Logger::Impl::Impl(LogLevel level, int savedErrno, const SourceFile& file, int line)
	: stream_(),
	level_(level),
	line_(line),
	basename_(file)
{
	formatTime();
	CurrentThread::tid();
	stream_ << T(CurrentThread::tidString(), CurrentThread::tidStringLength());
	stream_ << T(LogLevelName[level], 6);
	if (savedErrno != 0)
	{
		stream_ << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";
	}
}

void Logger::Impl::formatTime()
{
	struct timeval tv;
	time_t time;
	char str_t[26] = { 0 };
	gettimeofday(&tv, NULL);
	time = tv.tv_sec;
	struct tm* p_time = localtime(&time);
	strftime(str_t, 26, "%Y-%m-%d %H:%M:%S ", p_time);
	stream_ << str_t;
}

void Logger::Impl::finish()
{
	stream_ << " - " << basename_ << ':' << line_ << '\n';
}

Logger::Logger(SourceFile file, int line)
	: impl_(INFO, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
	: impl_(level, 0, file, line)
{
	impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level)
	: impl_(level, errno, file, line)
{
}


Logger::~Logger()
{
	impl_.finish();
	const LogStream::Buffer& buf(stream().buffer());
	g_output(buf.data(), buf.length());
	if (impl_.level_ == FATAL)
	{
		g_flush();
		abort();
	}
}

void Logger::setLogLevel(int level)
{
	switch (level)
	{
	case 1:
	{
		g_logLevel = TRACE;
		break;
	}
	case 2:
	{
		g_logLevel = DEBUG;
		break;
	}
	case 3:
	{
		g_logLevel = INFO;
		break;
	}
	case 4:
	{
		g_logLevel = WARN;
		break;
	}
	case 5:
	{
		g_logLevel = NONE;
		break;
	}
	default:
	{
		g_logLevel = INFO;
		break;
	}
	}
}

void Logger::setOutput(OutputFunc out)
{
	g_output = out;
}

void Logger::setFlush(FlushFunc flush)
{
	g_flush = flush;
}

