#ifndef __ENCODER_HPP__
#define __ENCODER_HPP__

#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

#include "common.hpp"
#include "Packet.hpp"
#include "SyncQueue.hpp"

namespace comm {

inline bool encode(
    const std::unique_ptr<uint8_t[]>& pData, const size_t& size,
    std::unique_ptr<uint8_t[]>& pEncodedData, size_t& encodedSize) {

    if ((pData) && validate_payload_size(size)) {
        encodedSize = sizeof(SF) + sizeof(size_t) + size + sizeof(EF);
        pEncodedData.reset(new uint8_t[encodedSize]);

        // Note: hard-coded to maximize performance!
        // 1. Start Frame
        uint8_t * internal_pointer = pEncodedData.get();
        *(internal_pointer++) = SF;

        // 2. Size (in bytes) of payload
        size_t tmp = size;
        *(internal_pointer++) = static_cast<uint8_t>(tmp & 0xFF);

        tmp = tmp >> 8;
        *(internal_pointer++) = static_cast<uint8_t>(tmp & 0xFF);

        tmp = tmp >> 8;
        *(internal_pointer++) = static_cast<uint8_t>(tmp & 0xFF);

        tmp = tmp >> 8;
        *(internal_pointer++) = static_cast<uint8_t>(tmp & 0xFF);

        // 3. Payload
        memcpy(internal_pointer, pData.get(), size);

        // 4. End Frame
        *(internal_pointer + size) = EF;

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

class Decoder {
 public:
    Decoder(): mState(E_SF) {}

    virtual ~Decoder() {
        resetBuffer();
    }

    void feed(uint8_t * pData, int size);

 private:
    inline void proceed(uint8_t b) {
        static int64_t timestampUs = -1L;
        switch (mState)
        {
            case E_SF:
                if (SF == b) {
                    resetBuffer();
                    timestampUs = get_monotonic_us();
                    mState = E_SIZE;
                } else {
                    // Discard
                }
                break;

            case E_SIZE:
            {
                static int size_byte_pos = 0;

                mPayloadSize |= (static_cast<size_t>(b) & 0x000000FF) << (size_byte_pos << 3);
                size_byte_pos++;

                if (sizeof(mPayloadSize) <= (uint16_t)size_byte_pos) {
                    size_byte_pos = 0;

                    if (validate_payload_size(mPayloadSize)) {
                        mpPayload.reset(new uint8_t[mPayloadSize]);
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

                mpPayload[payload_byte_pos++] = b;
                if (mPayloadSize <= payload_byte_pos) {
                    payload_byte_pos = 0;
                    mState = E_VALIDATION;
                }
            }
                break;

            case E_VALIDATION:
            {
                if (EF == b) {
                    // Save the frame
                    mDecodedQueue.enqueue(
                        Packet::create(
                            mpPayload, mPayloadSize, timestampUs
                        )
                    );
                }
            }
                // break;

            default:
                mState = E_SF;
                break;
        }
    }

    inline void resetBuffer() {
        mpPayload.reset();
        mPayloadSize = 0;
    }

    DECODING_STATES mState;

    size_t mPayloadSize;
    std::unique_ptr<uint8_t[]> mpPayload;

    dstruct::SyncQueue<Packet> mDecodedQueue;
};  // class Decoder

}   // namespace comm

#endif // __ENCODER_HPP__
