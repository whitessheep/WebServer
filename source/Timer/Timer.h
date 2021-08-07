//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.5.15
//Description:
#ifndef WEBSERVER_TIMER_H
#define WEBSERVER_TIMER_H


#include "HttpConn/HttpConnection.h" 
#include "Log/Logging.h"

#include <unordered_set>
#include <boost/circular_buffer.hpp>
#include <memory>


struct Entry 
{
	typedef std::weak_ptr<HttpConnection> WeakHttpConnectionPtr;
	typedef std::shared_ptr<HttpConnection> HttpConnectionPtr;

	explicit Entry(const WeakHttpConnectionPtr& weakConn)
	  : weakConn_(weakConn)
	{
	}

	~Entry();

	WeakHttpConnectionPtr weakConn_;
};

class TimerManager: noncopyable
{
public:
	TimerManager(int timelimit);
	
	
	void addTimer(const std::shared_ptr<HttpConnection>& conn);

	void extendTime(const std::shared_ptr<HttpConnection>& conn); 

	void handleEvent();

private:
	typedef std::shared_ptr<Entry> EntryPtr;
 	typedef std::weak_ptr<Entry> WeakEntryPtr;
  	typedef std::unordered_set<EntryPtr> Bucket;
  	typedef boost::circular_buffer<Bucket> WeakConnectionList;

	void dumpConnectionBuckets() const;    // for DEBUG
	static size_t getTime() ;
	static size_t const interval = 1000000;

  	WeakConnectionList connectionBuckets_;
	size_t expiredTime_;
};

#endif //WEBSERVER_TIMER_H
