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
    SyncQueue(int timeoutMs = DEFAULT_TIMEOUT_MS, size_t capLimit=DEFAULT_CAP_LIMIT) : 
        mTimeoutMs(timeoutMs), mCapLimit(capLimit) {}
    virtual ~SyncQueue() {}

    bool enqueue(std::unique_ptr<T>& pItem);
    bool enqueue(std::unique_ptr<T>&& pItem);
    bool dequeue(std::deque<std::unique_ptr<T>>& items, bool wait=true);

    void setTimeoutMs(int timeoutMs);
    void setCapLimit(size_t capLimit);

    static constexpr int DEFAULT_TIMEOUT_MS = 10;
    static constexpr size_t DEFAULT_CAP_LIMIT = 1024UL;

 private:
    std::deque<std::unique_ptr<T>> mQueue;
    std::mutex mMutex;
    std::condition_variable mCv;
    std::atomic<int> mTimeoutMs;
    std::atomic<std::size_t> mCapLimit;
};  // class SyncQueue

}   // namespace dstruct

#include "inline/SyncQueue.inl"

#endif // __SYNCQUEUE_HPP__
