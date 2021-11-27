#ifndef _MESSAGE_HPP_
#define _MESSAGE_HPP_

#include "common.hpp"

#include <cstring>
#include <memory>
#include <mutex>

#include <stdio.h>
#include <stdint.h>

namespace comm {

class IMessage {
public:
    virtual ~IMessage() {}
    virtual bool serialize(std::unique_ptr<uint8_t[]>& pSerializedData, int& serializedSize) = 0;
    virtual bool deserialize(const std::unique_ptr<uint8_t[]>& pSerializedData, const int& serializedSize) = 0;
}; // class IMessage

class Message: public IMessage {
public:
    Message() : mDataSize(0) {}
    Message(const Message &other)  noexcept {
        copy(other);
    }

    Message& operator = (const Message& other) {
        copy(other);
        return *this;
    }

    Message(Message&& other)  noexcept {
        *this = std::move(other);
    }

    Message& operator = (Message&& other) {
        if (&other != this) {
            this->pData = std::move(other.pData);
            this->mDataSize = other.mDataSize;

            other.mDataSize = 0;
        }

        return *this;
    }

    ~Message() { }

    bool update(const uint8_t* pData, const int& size);
    void dump();

    bool serialize(std::unique_ptr<uint8_t[]>& pSerializedData, int& serializedSize) override;
    bool deserialize(const std::unique_ptr<uint8_t[]>& pSerializedData, const int& serializedSize) override;

private:
    void copy(const Message &other) {
        std::lock_guard<std::mutex> lock(mDataMutex);
        this->mDataSize = other.mDataSize;
        this->pData.reset(new uint8_t[this->mDataSize]);
        memcpy(this->pData.get(), other.pData.get(), this->mDataSize);
    }

    std::unique_ptr<uint8_t[]> pData;
    int mDataSize;

    std::mutex mDataMutex;
}; // class NetMessage

class IObserver {
public:
    virtual ~IObserver() {}
    virtual void onRecv(const std::unique_ptr<Message>& pMessage) = 0;
}; // class IObserver

} // namespace comm

#endif // _MESSAGE_HPP_
