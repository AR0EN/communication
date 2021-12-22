#include "SyncQueue.hpp"

namespace dstruct {

template <class T>
inline bool SyncQueue<T>::enqueue(std::unique_ptr<T>& pItem) {
    bool result;

    std::lock_guard<std::mutex> lock(mMutex);
    result = mCapLimit > mQueue.size();
    if (result) {
        mQueue.push_back(std::move(pItem));
    }
    mCv.notify_one();

    return result;
}

template <class T>
inline bool SyncQueue<T>::enqueue(std::unique_ptr<T>&& pItem) {
    bool result;

    std::lock_guard<std::mutex> lock(mMutex);
    result = mCapLimit > mQueue.size();
    if (result) {
        mQueue.push_back(std::move(pItem));
    }
    mCv.notify_one();

    return result;
}

template <class T>
inline bool SyncQueue<T>::dequeue(std::deque<std::unique_ptr<T>>& items, bool wait) {
    bool result;

    if (wait) {
        std::unique_lock<std::mutex> lock(mMutex);

        if (mQueue.empty()) {
            mCv.wait_for(lock, std::chrono::milliseconds(mTimeoutMs));
        }

        result = !mQueue.empty();

        while (!mQueue.empty()) {
            items.push_back(std::move(mQueue.front()));
            mQueue.pop_front();
        }
    } else {
        std::lock_guard<std::mutex> lock(mMutex);

        result = !mQueue.empty();

        while (!mQueue.empty()) {
            items.push_back(std::move(mQueue.front()));
            mQueue.pop_front();
        }
    }

    return result;
}

template <class T>
inline void SyncQueue<T>::setTimeoutMs(int timeoutMs) {
    mTimeoutMs = timeoutMs;
}

template <class T>
inline void SyncQueue<T>::setCapLimit(size_t capLimit) {
    mCapLimit = capLimit;
}

}   // namespace dstruct
