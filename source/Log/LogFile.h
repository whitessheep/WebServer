#ifndef WEBSERVER_LOGFILE_H
#define WEBSERVER_LOGFILE_H
#include "base/Mutex.h"
#include "base/noncopyable.h"

#include <memory>
#include <sys/types.h>
#include <string>

class AppendFile;

class LogFile : noncopyable
{
public:
	LogFile(const std::string& basename,
		const std::string& path,
		off_t rollSize,
		int flushInterval = 3,
		int checkEveryN = 1024);
	~LogFile();

	void append(const char* logline, int len);
	void flush();
	bool rollFile();

private:
	static std::string getLogFileName(const std::string& basename, time_t* now);

	const std::string basename_;
	const std::string path_;
	const off_t rollSize_;
	const int flushInterval_;
	const int checkEveryN_;

	int count_;

	time_t startOfPeriod_;
	time_t lastRoll_;
	time_t lastFlush_;
	std::unique_ptr<AppendFile> file_;

	const static int kRollPerSeconds_ = 60 * 60 * 24;
	const static int pid_;
};

#endif  // WEBSERVER_LOGFILE_H
