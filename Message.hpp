#ifndef _MESSAGE_HPP_
#define _MESSAGE_HPP_

#include <stdio.h>
#include <stdint.h>

#include <memory>

#include "common.hpp"

namespace comm {

class Message {
public:
    virtual ~Message() {}
    virtual bool serialize(uint8_t *& pSerializedData, int& serializedSize) = 0;
    virtual bool deserialize(uint8_t * pSerializedData, int serializedSize) = 0;
}; // class Message

class NetMessage: public Message {
public:
    NetMessage() {
        mId = 0;
        mDataSize = 0;
        pData = nullptr;
    }

    bool update(uint16_t id, int size, uint8_t * pData) {
        if ((nullptr != pData) && (0 < size) && comm::ValidatePayloadSize(sizeof(mId) + size)) {
            this->mId = id;
            this->mDataSize = size;

            reset();
            this->pData = new uint8_t[this->mDataSize];
            memcpy(this->pData, pData, this->mDataSize);

            return true;
        } else {
            printf("Invalid parameters -> Could not update Message structure!\n");
            return false;
        }
    }

    ~NetMessage() {
        reset();
    }

    bool serialize(uint8_t *& pSerializedData, int& serializedSize) {
        if (nullptr == pData) {
            printf("Message is empty!\n");
            return false;
        }

        serializedSize = sizeof(mId) + mDataSize;
        pSerializedData = new uint8_t[serializedSize];

        int index = 0;
        memcpy(pSerializedData + index, &mId, sizeof(mId));
        index += sizeof(mId);

        memcpy(pSerializedData + index, pData, mDataSize);
        return true;
    }

    bool deserialize(uint8_t * pSerializedData, int serializedSize) {
        if ((nullptr != pSerializedData) && (0 < serializedSize)) {
            int index = 0;
            memcpy(&mId, pSerializedData + index, sizeof(mId));
            index += sizeof(mId);

            mDataSize = serializedSize - sizeof(mId);

            reset();
            pData = new uint8_t[mDataSize];
            printf("Allocated %d bytes for data buffer!\n", mDataSize);
            memcpy(pData, pSerializedData + index, mDataSize);

            return true;
        } else {
            printf("Invalid parameters -> Could not deserialize!\n");
            return false;
        }
    }

    void dump() {
        printf("Message:\n");
        printf("* Id: %u\n", mId);
        printf("* Data length: %d\n", mDataSize);
        printf("* Data: ");
        for (int i = 0; i < mDataSize; i++) {
            printf("0x%02X ", (int)pData[i] & 0x000000FF);
        }
        printf("\n\n");
    }

private:
    void reset() {
        if (nullptr != pData) {
            delete[] pData;
        }
    }

    uint16_t mId;
    int mDataSize;
    uint8_t * pData;
}; // class NetMessage

}; // namespace comm

#endif // _MESSAGE_HPP_
