#ifndef __SYNCQUEUE_HPP__
#define __SYNCQUEUE_HPP__

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>

namespace dstruct {

template <class T>
class SyncQueue {
 public:
    SyncQueue(int timeoutMs = 10) : mTimeoutMs(timeoutMs) {}
    virtual ~SyncQueue() {}

    void enqueue(const std::unique_ptr<T>& pItem) {
        std::lock_guard<std::mutex> lock(mMutex);
        mQueue.push(pItem);
        mCv.notify_one();
    }

    void dequeue(std::vector<std::unique_ptr<T>>& output) {
        std::unique_lock<std::mutex> lock(mMutex);

        if (mQueue.empty()) {
            mCv.wait_for(lock, std::chrono::milliseconds(mTimeoutMs));
        }

        while (!mQueue.empty()) {
            output.push_back(std::move(mQueue.front()));
            mQueue.pop();
        }
    }

    void setTimeoutMs(int timeoutMs) {
        mTimeoutMs = timeoutMs;
    }

 private:
    std::queue<std::unique_ptr<T>> mQueue;
    std::mutex mMutex;
    std::condition_variable mCv;
    std::atomic<int> mTimeoutMs;
};  // class SyncQueue

}   // namespace dstruct

#endif // __SYNCQUEUE_HPP__
