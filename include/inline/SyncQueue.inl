#include "SyncQueue.hpp"

namespace dstruct {

template <class T>
inline void SyncQueue<T>::enqueue(std::unique_ptr<T>& pItem) {
    std::lock_guard<std::mutex> lock(mMutex);
    mQueue.push(std::move(pItem));
    mCv.notify_one();
}

template <class T>
inline void SyncQueue<T>::enqueue(std::unique_ptr<T>&& pItem) {
    std::lock_guard<std::mutex> lock(mMutex);
    mQueue.push(std::move(pItem));
    mCv.notify_one();
}

template <class T>
inline bool SyncQueue<T>::dequeue(std::vector<std::unique_ptr<T>>& items) {
    bool result = false;
    std::unique_lock<std::mutex> lock(mMutex);

    if (mQueue.empty()) {
        mCv.wait_for(lock, std::chrono::milliseconds(mTimeoutMs));
    }

    result = !mQueue.empty();

    while (!mQueue.empty()) {
        items.push_back(std::move(mQueue.front()));
        mQueue.pop();
    }

    return result;
}

template <class T>
inline void SyncQueue<T>::setTimeoutMs(int timeoutMs) {
    mTimeoutMs = timeoutMs;
}

}   // namespace dstruct
