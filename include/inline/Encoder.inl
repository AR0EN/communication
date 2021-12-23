#include "Encoder.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Encoding
////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool comm::encode(
    const std::unique_ptr<uint8_t[]>& pData, const size_t& size, const uint16_t& tid,
    std::unique_ptr<uint8_t[]>& pEncodedData, size_t& encodedSize
) {

#ifdef DEBUG
    if (nullptr == pData) {
        LOGD("[%s][%d] Input buffer is empty!\n", __func__, __LINE__);
        return false;
    }

    if (!validate_payload_size(size)) {
        LOGD("[%s][%d] Input buffer size (%lu) is not acceptable!\n",
            __func__, __LINE__, static_cast<uint64_t>(size)
        );
        return false;
    }
#else   // DEBUG
    if ((pData) && validate_payload_size(size))
#endif  // DEBUG
    {
        encodedSize = SF_SIZE + SIZE_OF_TID + SIZE_OF_PAYLOAD_SIZE + size + EF_SIZE;
        pEncodedData.reset(new uint8_t[encodedSize]);

        // Note: hard-coded to maximize performance!
        // 1. Start Frame
        uint8_t * internal_pointer = pEncodedData.get();
        *(internal_pointer++) = SF;

        // 2. Transaction ID
        *(internal_pointer++) = static_cast<uint8_t>(tid & 0xFF);
        *(internal_pointer++) = static_cast<uint8_t>((tid >> 8) & 0xFF);

        // 3. Size (in bytes) of payload
        *(internal_pointer++) = static_cast<uint8_t>(size & 0xFF);
        *(internal_pointer++) = static_cast<uint8_t>((size >> 8) & 0xFF);
        *(internal_pointer++) = static_cast<uint8_t>((size >> 16) & 0xFF);
        *(internal_pointer++) = static_cast<uint8_t>((size >> 24) & 0xFF);

        // 4. Payload
        memcpy(internal_pointer, pData.get(), size);

        // 5. End Frame
        *(internal_pointer + size) = EF;

        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Decoding
////////////////////////////////////////////////////////////////////////////////////////////////////
inline void comm::Decoder::feed(const std::unique_ptr<uint8_t[]>& pdata, const size_t& size) {
    LOGD("[%s][%d] Feed %lu bytes!\n", __func__, __LINE__, size);
    for (size_t i = 0; i < size; i++) {
        proceed(pdata[i]);
    }
}

inline bool comm::Decoder::dequeue(std::deque<std::unique_ptr<Packet>>& pPackets, bool wait) {
    return mDecodedQueue.dequeue(pPackets, wait);
}

inline void comm::Decoder::proceed(const uint8_t& b) {
    static int64_t timestampUs = -1L;
    switch (mState)
    {
        case E_SF:
            if (SF == b) {
                resetBuffer();
                timestampUs = get_elapsed_realtime_us();
                mState = E_TID;
            } else {
                // Discard
                LOGE("[%s][%d] Expected 0x%02X but received 0x%02X!\n",
                    __func__, __LINE__, static_cast<unsigned int>(SF), static_cast<unsigned int>(b)
                );
            }
            break;

        case E_TID:
        {
            static int tid_byte_pos = 0;
            static int delta = 0;

            LOGD("[%s][%d] TID byte %d -> shift %d bits!\n",
                __func__, __LINE__,
                tid_byte_pos, (tid_byte_pos << 3)
            );
            mTransactionId |= (static_cast<int>(b) & 0xFF) << (tid_byte_pos++ << 3);

            if (SIZE_OF_TID <= static_cast<size_t>(tid_byte_pos)) {
                tid_byte_pos = 0;

                if (0 <= mCachedTransactionId) {
                    if (mCachedTransactionId <= mTransactionId) {
                        delta = mTransactionId - mCachedTransactionId;
                    } else {
                        // Carry-over
                        delta = (MAX_VALUE_OF_TID - mCachedTransactionId) + mTransactionId;
                    }

                    if (0 == delta) {
                        LOGE("[%s][%d] Duplicated Transaction ID: %d -> %d!\n",
                            __func__, __LINE__, mCachedTransactionId, mTransactionId
                        );
                    } else if (1 < delta) {
                        LOGE("[%s][%d] Lost packets between (%d;%d)!\n",
                            __func__, __LINE__, mCachedTransactionId, mTransactionId
                        );
                    } else {
                        LOGD("[%s][%d] Transaction ID: %d -> %d!\n",
                            __func__, __LINE__, mCachedTransactionId, mTransactionId
                        );
                    }
                } else {
                    LOGD("[%s][%d] Received 1st packet with Transaction ID: %d\n",
                         __func__, __LINE__, mTransactionId
                    );
                }

                mCachedTransactionId = mTransactionId;

                mState = E_SIZE;
            }
        }
            break;

        case E_SIZE:
        {
            static size_t size_byte_pos = 0;
            LOGD("[%s][%d] Size byte %zu -> shift %zu bits!\n",
                __func__, __LINE__,
                size_byte_pos, (size_byte_pos << 3)
            );
            mPayloadSize |= (static_cast<size_t>(b) & 0xFFUL) << (size_byte_pos++ << 3);

            if (SIZE_OF_PAYLOAD_SIZE <= size_byte_pos) {
                size_byte_pos = 0;

                if (validate_payload_size(mPayloadSize)) {
                    mpPayload.reset(new uint8_t[mPayloadSize]);
                    mState = E_PAYLOAD;
                    LOGD("[%s][%d] Payload size: %zu (bytes)!\n", __func__, __LINE__, mPayloadSize);
                } else {
                    // Invalid payload size!
                    mState = E_SF;
                    LOGE("[%s][%d] Invalid payload size: %zu!\n", __func__, __LINE__, mPayloadSize);
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
                if (!mDecodedQueue.enqueue(Packet::create(mpPayload, mPayloadSize, timestampUs))) {
                    LOGE("Decoder Queue is full!\n");
                }

                LOGD("[%s][%d] Decoded a packet with %lu bytes payload at %ld (us)!\n",
                    __func__, __LINE__, mPayloadSize, timestampUs
                );
            } else {
                // Discard
                LOGE("[%s][%d] Expected 0x%02X but received 0x%02X!\n",
                    __func__, __LINE__, EF, b
                );
            }
        }
            // break;

        default:
            mState = E_SF;
            break;
    }
}

inline void comm::Decoder::resetBuffer() {
    mpPayload.reset();
    mPayloadSize = 0UL;
    mTransactionId = 0;
}
