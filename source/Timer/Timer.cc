#include "Timer.h"
#include "HttpConn/HttpConnection.h"
#include "Log/Logging.h"

#include "sys/time.h"

Entry::~Entry()
{
  HttpConnectionPtr conn = weakConn_.lock();
  if (conn)
  {
	LOG_DEBUG << "conn time out fd = " << conn->fd();
	conn->shutdown();
  }
}


TimerManager::TimerManager(int timelimit)
	:connectionBuckets_(timelimit),
	 expiredTime_(getTime() + interval)
{ connectionBuckets_.resize(timelimit); }

void TimerManager::addTimer(const std::shared_ptr<HttpConnection>& conn) {
	EntryPtr entry(new Entry(conn));
	connectionBuckets_.back().insert(entry);
	WeakEntryPtr weakentry(entry);
	conn->setTimer(weakentry);
}

void TimerManager::extendTime(const std::shared_ptr<HttpConnection>& conn) {
	WeakEntryPtr weakentry(conn->getTimer());
	EntryPtr entry(weakentry.lock());
	if (entry) {
		connectionBuckets_.back().insert(entry);
	}
}

void TimerManager::handleEvent() {
	size_t now = getTime();
	if (now >= expiredTime_) {
		connectionBuckets_.push_back(Bucket());
		// dumpConnectionBuckets();
		expiredTime_ = now + interval;
	}
}

void TimerManager::dumpConnectionBuckets() const    
{
	std::cout << "size = " << connectionBuckets_.size() << std::endl;
	int idx = 0;
	for (WeakConnectionList::const_iterator bucketI = connectionBuckets_.begin();
		 bucketI != connectionBuckets_.end();
		 ++bucketI, ++idx) {
		const Bucket& bucket = *bucketI;
		printf("[%d] len = %zd : ", idx, bucket.size());
		for (const auto& it : bucket) {
			bool connectionDead = it->weakConn_.expired();
			printf("%p(%ld)%s, ", &(*it), it.use_count(),
			connectionDead ? " DEAD" : "");
		}
		puts("");
	}
}

size_t TimerManager::getTime() {
	struct timeval now;
	gettimeofday(&now, NULL);
	return (now.tv_sec  * 1000000) + now.tv_usec;
}
