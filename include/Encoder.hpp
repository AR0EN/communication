#ifndef _ENCODER_HPP_
#define _ENCODER_HPP_

#include "common.hpp"

#include <memory>
#include <vector>

#include <stdint.h>
#include <string.h>

namespace comm {

inline bool encode(
    const std::unique_ptr<uint8_t[]>& pData, const csize_t& size,
    std::unique_ptr<uint8_t[]>& pEncodedData, csize_t& encodedSize) {

    if ((pData) && ValidatePayloadSize(size)) {
        encodedSize = sizeof(SF) + sizeof(csize_t) + size + sizeof(EF);
        pEncodedData.reset(new uint8_t[encodedSize]);

        int index = 0;
        *(pEncodedData.get() + index++) = SF;
        memcpy(pEncodedData.get() + index, &size, sizeof(size));
        index += sizeof(encodedSize);

        memcpy(pEncodedData.get() + index, pData.get(), size);
        index += size;

        *(pEncodedData.get() + index) = EF;

        return true;
    }

    return false;
}

enum DECODING_STATES {
    E_SF,
    E_SIZE,
    E_PAYLOAD,
    E_VALIDATION
};

class DecodingObserver {
public:
    virtual ~DecodingObserver() {}
    virtual void onComplete(const std::unique_ptr<uint8_t[]>& pData, const csize_t& size) = 0;
};

class Decoder {
public:
    Decoder() {
        mState = E_SF;
        pPayload = nullptr;
    }

    ~Decoder() {
        resetBuffer();
    }

    void feed(uint8_t * pData, int size);

    void subscribe(const std::shared_ptr<DecodingObserver>& pObserver);
    // void unsubscribe();

private:
    inline void proceed(uint8_t b) {
        switch (mState)
        {
        case E_SF:
            if (SF == b) {
                resetBuffer();
                mState = E_SIZE;
            } else {
                // Discard
            }
            break;

        case E_SIZE:
        {
            static int size_byte_pos = 0;

            mPayloadSize |= ((csize_t)b & 0x000000FF) << size_byte_pos++;

            if (sizeof(mPayloadSize) <= (uint16_t)size_byte_pos) {
                size_byte_pos = 0;

                if (ValidatePayloadSize(mPayloadSize)) {
                    pPayload = new uint8_t[mPayloadSize];
                    mState = E_PAYLOAD;
                } else {
                    // Invalid payload size!
                    mState = E_SF;
                }
            }
        }
            break;

        case E_PAYLOAD:
        {
            static int payload_byte_pos = 0;

            pPayload[payload_byte_pos++] = b;
            if (mPayloadSize <= payload_byte_pos) {
                payload_byte_pos = 0;
                mState = E_VALIDATION;
            }
        }
            break;

        case E_VALIDATION:
        {
            if (EF == b) {
                notify();
            }
        }
            // break;

        default:
            mState = E_SF;
            break;
        }
    }

    void notify();

    void resetBuffer() {
        if (nullptr != pPayload) {
            delete[] pPayload;
            pPayload = nullptr;
        }

        mPayloadSize = 0;
    }

    DECODING_STATES mState;
    csize_t mPayloadSize;
    uint8_t * pPayload;

    std::vector<std::shared_ptr<DecodingObserver>> mObservers;

};  // class Decoder

};  // namespace comm

#endif // _ENCODER_HPP_
