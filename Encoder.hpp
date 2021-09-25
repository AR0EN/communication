#ifndef _ENCODER_HPP_
#define _ENCODER_HPP_

#include <stdint.h>
#include <string.h>

#include <memory>
#include <vector>

namespace comm {

constexpr uint8_t SF = 0xF0;
constexpr uint8_t EF = 0x0F;

constexpr int32_t MAX_PAYLOAD_SIZE = 1024;

enum DECODING_STATES {
    E_SF,
    E_SIZE,
    E_PAYLOAD,
    E_VALIDATION
};

class DecodingObserver {
public:
    virtual int getId() = 0;
    virtual void onComplete(const std::shared_ptr<uint8_t>& pData, int size) = 0;
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

    void subscribe(std::shared_ptr<DecodingObserver>& pObserver);

    static bool validatePayloadSize(int32_t payloadSize) {
        return (0 < payloadSize) && (MAX_PAYLOAD_SIZE >= payloadSize);
    }

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

            mPayloadSize |= ((int32_t)b & 0x000000FF) << size_byte_pos++;

            if (sizeof(mPayloadSize) <= (uint16_t)size_byte_pos) {
                size_byte_pos = 0;

                if (validatePayloadSize(mPayloadSize)) {
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
    int32_t mPayloadSize;
    uint8_t * pPayload;

    std::vector<std::shared_ptr<DecodingObserver>> mObservers;

};  // class Decoder

};  // namespace comm

#endif // _ENCODER_HPP_
