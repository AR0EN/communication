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
    virtual bool serialize(uint8_t *& pSerializedData, int& serializedSize) = 0;
    virtual bool deserialize(uint8_t * pSerializedData, int serializedSize) = 0;
}; // class IMessage

class Message: public IMessage {
public:
    Message() : mDataSize(0), pData(nullptr) {}
    Message(const Message &other)  noexcept {
        copy(other);
    }

    Message& operator = (const Message& other) {
        reset();
        copy(other);
        return *this;
    }

    Message(Message&& other)  noexcept {
        *this = std::move(other);
    }

    Message& operator = (Message&& other) {
        if (&other != this) {
            reset();
            this->pData = other.pData;
            this->mDataSize = other.mDataSize;

            other.pData = nullptr;
            other.mDataSize = 0;
        }

        return *this;
    }

    ~Message() {
        reset();
    }

    bool update(uint8_t * pData, int size);
    void dump();

    bool serialize(uint8_t *& pSerializedData, int& serializedSize) override;
    bool deserialize(uint8_t * pSerializedData, int serializedSize) override;

private:
    void copy(const Message &other) {
        std::lock_guard<std::mutex> lock(mDataMutex);
        this->mDataSize = other.mDataSize;
        this->pData = new uint8_t[this->mDataSize];
        memcpy(this->pData, other.pData, this->mDataSize);
    }

    void reset() {
        if (nullptr != pData) {
            delete[] pData;
            pData = nullptr;
        }
    }

    uint8_t * pData;
    int mDataSize;

    std::mutex mDataMutex;
}; // class NetMessage

class IObserver {
public:
    virtual ~IObserver() {}
    virtual void onRecv(const std::shared_ptr<Message>& pMessage) = 0;
}; // class IObserver

} // namespace comm

#endif // _MESSAGE_HPP_
