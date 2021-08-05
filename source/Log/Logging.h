//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.2
//Description:
#ifndef WEBSERVER_LOGGING_H
#define WEBSERVER_LOGGING_H

#include "LogStream.h"

class Logger
{
public:
	enum LogLevel
	{
		TRACE,
		DEBUG,
		INFO,
		WARN,
		ERROR,
		FATAL,
		NONE,
	};

	class SourceFile
	{
	public:
		template<int N>
		SourceFile(const char(&arr)[N])
			: data_(arr),
			size_(N - 1)
		{
			const char* slash = strrchr(data_, '/');
			if (slash)
			{
				data_ = slash + 1;
				size_ -= static_cast<int>(data_ - arr);
			}
		}

		explicit SourceFile(const char* filename)
			: data_(filename)
		{
			const char* slash = strrchr(filename, '/');
			if (slash)
			{
				data_ = slash + 1;
			}
			size_ = static_cast<int>(strlen(data_));
		}

		const char* data_;
		int size_;
	};

	Logger(SourceFile file, int line);
	Logger(SourceFile file, int line, LogLevel level);
	Logger(SourceFile file, int line, LogLevel level, const char* func);
	~Logger();

	LogStream& stream() { return impl_.stream_; }

	static LogLevel logLevel();
	static void setLogLevel(int level);

	typedef void (*OutputFunc)(const char* msg, int len);
	typedef void (*FlushFunc)();
	static void setOutput(OutputFunc);
	static void setFlush(FlushFunc);

private:

	class Impl
	{
	public:
		typedef Logger::LogLevel LogLevel;
		Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
		void formatTime();
		void finish();

		LogStream stream_;
		LogLevel level_;
		int line_;
		SourceFile basename_;
	};

	Impl impl_;

};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel()
{
	return g_logLevel;
}
#define LOG_TRACE if (Logger::logLevel() <= Logger::TRACE) \
  Logger(__FILE__, __LINE__, Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (Logger::logLevel() <= Logger::DEBUG) \
  Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
#define LOG_INFO if (Logger::logLevel() <= Logger::INFO) \
  Logger(__FILE__, __LINE__).stream()
#define LOG_WARN if (Logger::logLevel() <= Logger::WARN) \
  Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR if (Logger::logLevel() <= Logger::WARN) \
  Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL if (Logger::logLevel() <= Logger::WARN) \
  Logger(__FILE__, __LINE__, Logger::FATAL).stream()

const char* strerror_tl(int savedErrno);



#endif // WEBSERVER_LOGGING_H
