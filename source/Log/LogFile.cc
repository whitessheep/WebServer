#include "LogFile.h"
#include "FileUtil.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <string>
#include <unistd.h>
using std::string;

const int LogFile::pid_ = getpid();

LogFile::LogFile(const string& basename,
	const string& path,
	off_t rollSize,
	int flushInterval,
	int checkEveryN)
	: basename_(basename),
	path_(path),
	rollSize_(rollSize),
	flushInterval_(flushInterval),
	checkEveryN_(checkEveryN),
	count_(0),
	startOfPeriod_(0),
	lastRoll_(0),
	lastFlush_(0)
{
	assert(basename.find('/') == string::npos);
	rollFile();
}

LogFile::~LogFile() = default;

void LogFile::flush()
{
	file_->flush();
}

void LogFile::append(const char* logline, int len)
{
	file_->append(logline, len);

	if (file_->writtenBytes() > rollSize_)
	{
		rollFile();
	}
	else
	{
		++count_;
		if (count_ >= checkEveryN_)
		{
			count_ = 0;
			time_t now = ::time(NULL);
			time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
			if (thisPeriod_ != startOfPeriod_)
			{
				rollFile();
			}
			else if (now - lastFlush_ > flushInterval_)
			{
				lastFlush_ = now;
				file_->flush();
			}
		}
	}
}

bool LogFile::rollFile()
{
	time_t now = 0;
	string filename = getLogFileName(basename_, &now);
	time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

	if (now > lastRoll_)
	{
		lastRoll_ = now;
		lastFlush_ = now;
		startOfPeriod_ = start;
		file_.reset(new AppendFile(filename));
		return true;
	}
	return false;
}

string LogFile::getLogFileName(const string & basename, time_t * now)
{
	string filename;
	filename.reserve(basename.size() + 64);
	filename = basename;

	char timebuf[32];
	struct tm tm;
	*now = time(NULL);
	localtime_r(now, &tm);
	strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
	filename += timebuf;


	char pidbuf[32];
	snprintf(pidbuf, sizeof pidbuf, ".%d", pid_);
	filename += pidbuf;

	filename += ".log";

	return filename;
}

