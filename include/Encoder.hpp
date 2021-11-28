#ifndef __ENCODER_HPP__
#define __ENCODER_HPP__

#include <cstdint>
#ifdef DEBUG
#include <cstdio>
#endif  // DEBUG
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

#ifdef DEBUG
    if (nullptr == pData) {
        printf("[%s][%d] Input buffer is empty!\n", __func__, __LINE__);
        return false;
    }

    if (!validate_payload_size(size)) {
        printf("[%s][%d] Input buffer size (%lu) is not acceptable!\n",
            __func__, __LINE__, static_cast<uint64_t>(size)
        );
        return false;
    }
#else
    if ((pData) && validate_payload_size(size))
#endif  // DEBUG
    {
        encodedSize = SF_SIZE + SIZE_OF_PAYLOAD_SIZE + size + EF_SIZE;
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

    inline void feed(const std::unique_ptr<uint8_t[]>& pdata, const size_t& size) {
#ifdef DEBUG
        printf("[%s][%d] Feed %lu bytes!\n", __func__, __LINE__, size);
#endif  // DEBUG
        for (size_t i = 0; i < size; i++) {
            proceed(pdata[i]);
        }
    }

    bool dequeue(std::vector<std::unique_ptr<Packet>>& packets) {
        return mDecodedQueue.dequeue(packets);
    }

 private:
    inline void proceed(const uint8_t& b) {
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
#ifdef DEBUG
                    printf("[%s][%d] Expected 0x%02X but received 0x%02X!\n",
                        __func__, __LINE__, SF, b
                    );
#endif  // DEBUG
                }
                break;

            case E_SIZE:
            {
                static size_t size_byte_pos = 0;
#ifdef DEBUG
                printf("[%s][%d] Size byte %lu -> shift %lu bits!\n",
                    __func__, __LINE__,
                    static_cast<uint64_t>(size_byte_pos), static_cast<uint64_t>(size_byte_pos << 3)
                );
#endif  // DEBUG
                mPayloadSize |= (static_cast<size_t>(b) & 0x000000FF) << (size_byte_pos++ << 3);

                if (SIZE_OF_PAYLOAD_SIZE <= size_byte_pos) {
                    size_byte_pos = 0;

                    if (validate_payload_size(mPayloadSize)) {
                        mpPayload.reset(new uint8_t[mPayloadSize]);
                        mState = E_PAYLOAD;
#ifdef DEBUG
                        printf("[%s][%d] Payload size: %lu!\n",
                            __func__, __LINE__, static_cast<uint64_t>(mPayloadSize)
                        );
#endif  // DEBUG
                    } else {
                        // Invalid payload size!
                        mState = E_SF;
#ifdef DEBUG
                        printf("[%s][%d] Invalid payload size: %lu!\n",
                            __func__, __LINE__, static_cast<uint64_t>(mPayloadSize)
                        );
#endif  // DEBUG
                    }
                }
            }
                break;

            case E_PAYLOAD:
            {
                static size_t payload_byte_pos = 0;

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
#ifdef DEBUG
                    printf("[%s][%d] Decoded a packet with %lu bytes payload at %ld (us)!\n",
                        __func__, __LINE__, mPayloadSize, timestampUs
                    );
#endif  // DEBUG
                } else {
                    // Discard
#ifdef DEBUG
                    printf("[%s][%d] Expected 0x%02X but received 0x%02X!\n",
                        __func__, __LINE__, EF, b
                    );
#endif  // DEBUG
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
