#pragma once

#include <thread>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <condition_variable>

#include "tools/debug.h"
#include <typeinfo>
/*
 * Base class for queues that need to be safe for concurrent access
 * This class is implemented with IPC and C++ container. The condition variable
 * and mutex takes care of locking/synchronization and std::queue preserves
 * the smart pointer mechanics so we can do zero-copy.
*/

template <class T>
class ProtectedQueue
{
protected:
	std::condition_variable cv_;
	std::mutex cv_lock_;
	std::deque<std::shared_ptr<T>> q_;
    bool pdebug_;
    int size_;

public:
    ProtectedQueue(int size = 10000, bool debug = false)
    {
        size_ = size,
        pdebug_ = debug;
    }
	void Put(std::shared_ptr<T> entry);
    void PutFront(std::shared_ptr<T> entry);
    const std::shared_ptr<T> Get(void);
    void Flush(void);
    uint16_t qsize() {return q_.size();}
};

template<class T>
const std::shared_ptr<T> ProtectedQueue<T>::Get(void) {
    if (pdebug_) debug("GET requesting lock!\n");
    std::unique_lock<std::mutex> lock(cv_lock_);

    while (q_.empty()) {
        if (pdebug_) debug("GET wait for cv!\n");
        cv_.wait(lock);
        if (pdebug_) debug("GET cv signaled!\n");
    }

    if (pdebug_) debug("GET retrieve from Q\n");
    auto retval = q_.front();
    q_.pop_front();
    return retval;
}

template<class T>
void ProtectedQueue<T>::Put(std::shared_ptr<T> entry) {
    if(qsize() < size_) {
        {
            if (pdebug_) debug("PUT requesting lock!\n");
        	std::lock_guard<std::mutex> lock(cv_lock_);

        	if (pdebug_) debug("PUT lock acquired\n");
        	q_.push_back(entry);
        }
        cv_.notify_one();
        if (pdebug_) debug("PUT notified cv\n");
    } else {
        debug("QSIZE TOO BIG, REJECTING PUT: %d\n", qsize());
    }
}

template<class T>
void ProtectedQueue<T>::PutFront(std::shared_ptr<T> entry) {
    std::unique_lock<std::mutex> lock(cv_lock_);
    q_.push_front(entry);
    cv_.notify_one();
}

template<class T>
void ProtectedQueue<T>::Flush(void) {
    std::unique_lock<std::mutex> lock(cv_lock_);
    while (!q_.empty()) {
        q_.pop_front();
    }
}
