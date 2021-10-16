#include "Message.hpp"

#include <cstring>

namespace comm {

bool Message::serialize(std::unique_ptr<uint8_t[]>& pSerializedData, int& serializedSize) {
    if (!pData) {
        printf("Message is empty!\n");
        return false;
    }

    std::lock_guard<std::mutex> lock(mDataMutex);
    pSerializedData.reset(new uint8_t[mDataSize]);
    memcpy(pSerializedData.get(), pData.get(), mDataSize);
    serializedSize = mDataSize;

    return true;
}

bool Message::deserialize(const std::unique_ptr<uint8_t[]>& pSerializedData, const int& serializedSize) {
    if ((pSerializedData) && (0 < serializedSize)) {
        std::lock_guard<std::mutex> lock(mDataMutex);

        mDataSize = serializedSize;

        pData.reset(new uint8_t[mDataSize]);
        printf("Allocated %d bytes for data buffer!\n", mDataSize);
        memcpy(pData.get(), pSerializedData.get(), mDataSize);

        return true;
    } else {
        printf("Invalid parameters -> Could not deserialize!\n");
        return false;
    }
}

bool Message::update(const uint8_t * pData, const int& size) {
    if ((nullptr != pData) && comm::ValidatePayloadSize(size)) {
        std::lock_guard<std::mutex> lock(mDataMutex);
        this->mDataSize = size;

        this->pData.reset(new uint8_t[this->mDataSize]);
        memcpy(this->pData.get(), pData, this->mDataSize);

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
            printf("0x%02X ", (int)*(pData.get() + i) & 0x000000FF);
        }

        if (i < mDataSize) {
            printf(" ...");
        }
    }

    printf("\n\n");
}

} // namespace comm
