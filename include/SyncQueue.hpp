#ifndef __SYNCQUEUE_HPP__
#define __SYNCQUEUE_HPP__

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>

namespace dstruct {

template <class T>
class SyncQueue {
 public:
    SyncQueue(int timeoutMs = 10) : mTimeoutMs(timeoutMs) {}
    virtual ~SyncQueue() {}

    void enqueue(std::unique_ptr<T>& pItem);
    void enqueue(std::unique_ptr<T>&& pItem);
    bool dequeue(std::deque<std::unique_ptr<T>>& items, bool wait=true);

    void setTimeoutMs(int timeoutMs);

 private:
    std::deque<std::unique_ptr<T>> mQueue;
    std::mutex mMutex;
    std::condition_variable mCv;
    std::atomic<int> mTimeoutMs;
};  // class SyncQueue

}   // namespace dstruct

#include "inline/SyncQueue.inl"

#endif // __SYNCQUEUE_HPP__
