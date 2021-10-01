#include "Message.hpp"

#include <cstring>

namespace comm {

bool Message::serialize(uint8_t *& pSerializedData, int& serializedSize) {
    if (nullptr == pData) {
        printf("Message is empty!\n");
        return false;
    }

    std::lock_guard<std::mutex> lock(mDataMutex);
    pSerializedData = new uint8_t[mDataSize];
    memcpy(pSerializedData, pData, mDataSize);
    serializedSize = mDataSize;

    return true;
}

bool Message::deserialize(uint8_t * pSerializedData, int serializedSize) {
    if ((nullptr != pSerializedData) && (0 < serializedSize)) {
        std::lock_guard<std::mutex> lock(mDataMutex);

        mDataSize = serializedSize;

        reset();
        pData = new uint8_t[mDataSize];
        printf("Allocated %d bytes for data buffer!\n", mDataSize);
        memcpy(pData, pSerializedData, mDataSize);

        return true;
    } else {
        printf("Invalid parameters -> Could not deserialize!\n");
        return false;
    }
}

bool Message::update(uint8_t * pData, int size) {
    if ((nullptr != pData) && (0 < size) && comm::ValidatePayloadSize(size)) {
        std::lock_guard<std::mutex> lock(mDataMutex);
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

void Message::dump() {
    printf("Message:\n");
    printf("* Data length: %d\n", mDataSize);
    printf("* Data: ");

    {
        int i;
        std::lock_guard<std::mutex> lock(mDataMutex);
        for (i = 0; (i < mDataSize) && (i < 8); i++) {
            printf("0x%02X ", (int)pData[i] & 0x000000FF);
        }

        if (i < mDataSize) {
            printf(" ...");
        }
    }

    printf("\n\n");
}

} // namespace comm
