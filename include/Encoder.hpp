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

bool encode(
    const std::unique_ptr<uint8_t[]>& pData, const size_t& size,
    std::unique_ptr<uint8_t[]>& pEncodedData, size_t& encodedSize
);

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

    void feed(const std::unique_ptr<uint8_t[]>& pdata, const size_t& size);

    bool dequeue(std::vector<std::unique_ptr<Packet>>& pPackets);

 private:
    void proceed(const uint8_t& b);

    void resetBuffer();

    DECODING_STATES mState;

    size_t mPayloadSize;
    std::unique_ptr<uint8_t[]> mpPayload;

    dstruct::SyncQueue<Packet> mDecodedQueue;
};  // class Decoder

}   // namespace comm

#include "Encoder.inl"

#endif // __ENCODER_HPP__
