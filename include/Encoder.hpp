#ifndef __ENCODER_HPP__
#define __ENCODER_HPP__

#include <cstdint>

#include <cstring>
#include <deque>
#include <memory>

#include "common.hpp"
#include "Packet.hpp"
#include "SyncQueue.hpp"

namespace comm {

bool encode(
    const std::unique_ptr<uint8_t[]>& pData, const size_t& size, const uint16_t& tid,
    std::unique_ptr<uint8_t[]>& pEncodedData, size_t& encodedSize
);

enum DECODING_STATES {
    E_SF,
    E_TID,
    E_SIZE,
    E_PAYLOAD,
    E_VALIDATION
};

class Decoder {
 public:
    Decoder(): mState(E_SF), mCachedTransactionId(-1) {}
    virtual ~Decoder() { resetBuffer(); }

    void feed(const std::unique_ptr<uint8_t[]>& pdata, const size_t& size);
    bool dequeue(std::deque<std::unique_ptr<Packet>>& pPackets, bool wait=true);

 private:
    void proceed(const uint8_t& b);
    void resetBuffer();

    DECODING_STATES mState;

    size_t mPayloadSize;
    std::unique_ptr<uint8_t[]> mpPayload;

    dstruct::SyncQueue<Packet> mDecodedQueue;
    int mTransactionId;
    int mCachedTransactionId;
};  // class Decoder

}   // namespace comm

#include "inline/Encoder.inl"

#endif // __ENCODER_HPP__
