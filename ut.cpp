#include <stdio.h>
#include <stdlib.h>

#include <memory>

#include "test_vectors.hpp"

#include "Encoder.hpp"
#include "Message.hpp"

class TestMessage: public comm::Message {
public:
    TestMessage() {
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

    ~TestMessage() {
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
};

class Observer : public comm::DecodingObserver {
    public:
    int getId() {
        return 3188;
    }

    void onComplete(const std::shared_ptr<uint8_t>& pData, int size) {
        if (pData) {
            uint8_t * tmp = pData.get();

            TestMessage msg;
            msg.deserialize(tmp, size);

            printf("Received:\n");
            msg.dump();
            printf("\n");
        }
    }
};

int main() {
    comm::Decoder decoder;

    std::shared_ptr<comm::DecodingObserver> pObserver = std::make_shared<Observer>();
    decoder.subscribe(pObserver);

    uint8_t * pSerializedData;
    uint8_t * pEncodedData;

    int serializedSize;
    int encodedSize;
    for (int i = 0; i < vectors.size(); i++) {
        TestMessage msg;
        msg.update(vectors_identifiers[i], vectors_sizes[i], vectors[i]);
        printf("Original:\n");
        msg.dump();

        msg.serialize(pSerializedData, serializedSize);

        if (nullptr != pSerializedData) {
            comm::encode(pSerializedData, serializedSize, pEncodedData, encodedSize);

            if (nullptr != pEncodedData) {
                decoder.feed(pEncodedData, encodedSize);
                delete[] pEncodedData;
            }

            delete[] pSerializedData;
        }
    }

    return 0;
}
