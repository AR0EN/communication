#ifndef _OBSERVER_IMPL_HPP_
#define _OBSERVER_IMPL_HPP_

#include "Message.hpp"

class MessageImpl: public comm::IMessage {
public:
    MessageImpl() : mDataSize(0) {}
    MessageImpl(const MessageImpl &other)  noexcept {
        copy(other);
    }

    MessageImpl& operator = (const MessageImpl& other) {
        copy(other);
        return *this;
    }

    MessageImpl(MessageImpl&& other)  noexcept {
        *this = std::move(other);
    }

    MessageImpl& operator = (MessageImpl&& other) {
        if (&other != this) {
            this->pData = std::move(other.pData);
            this->mDataSize = other.mDataSize;

            other.mDataSize = 0;
        }

        return *this;
    }

    ~MessageImpl() { }

    bool update(const uint8_t* pData, const int& size);
    void dump() const;

    bool serialize(std::unique_ptr<uint8_t[]>& pSerializedData, int& serializedSize) override;
    bool deserialize(const std::unique_ptr<uint8_t[]>& pSerializedData, const int& serializedSize) override;

private:
    void copy(const MessageImpl &other) {
        std::lock_guard<std::mutex> lock(mDataMutex);
        this->mDataSize = other.mDataSize;
        this->pData.reset(new uint8_t[this->mDataSize]);
        memcpy(this->pData.get(), other.pData.get(), this->mDataSize);
    }

    std::unique_ptr<uint8_t[]> pData;
    int mDataSize;

    mutable std::mutex mDataMutex;
}; // class MessageImpl

class ObserverImpl : public comm::IObserver<MessageImpl> {
public:
    ObserverImpl() {}
    ~ObserverImpl() {}
    void onRecv(const MessageImpl& message) override {
        message.dump();
    }
}; // class ObserverImpl

#endif // _OBSERVER_IMPL_HPP_