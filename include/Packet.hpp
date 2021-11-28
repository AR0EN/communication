#ifndef __PACKET_HPP__
#define __PACKET_HPP__

#include <cstdint>
#include <cstring>
#include <memory>

#include "common.hpp"

namespace comm {

class Packet {
 public:
    Packet(Packet&& other) {
        mpPayload = std::move(other.mpPayload);
        mPayloadSize = other.mPayloadSize;
        other.mPayloadSize = 0L;

        mTimestampUs = other.mTimestampUs;
        other.mTimestampUs = -1L;
    }

    Packet& operator=(Packet&& other) {
        if (this != &other) {
            mpPayload = std::move(other.mpPayload);
            mPayloadSize = other.mPayloadSize;
            other.mPayloadSize = 0L;

            mTimestampUs = other.mTimestampUs;
            other.mTimestampUs = -1L;
        }

        return *this;
    }

    Packet(const Packet&) = delete;
    Packet& operator=(const Packet&) = delete;

    virtual ~Packet() {}

    static std::unique_ptr<Packet> create(
        const std::unique_ptr<uint8_t[]>& pPayload,
        const size_t& payloadSize,
        const int64_t& timestampUs=-1) {

        if ((pPayload) && validate_payload_size(payloadSize)) {
            return std::unique_ptr<Packet>(
                new Packet(pPayload, payloadSize, timestampUs));
        } else {
            return std::unique_ptr<Packet>(nullptr);
        }
    }

    const std::unique_ptr<uint8_t[]>& getPayload() {
        return mpPayload;
    }

    const size_t& getPayloadSize() {
        return mPayloadSize;
    }

    const int64_t& getTimestampUs() {
        return mTimestampUs;
    }

 protected:
    Packet(
        const std::unique_ptr<uint8_t[]>& pPayload,
        const size_t& payloadSize,
        const int64_t& timestampUs) {

        mTimestampUs = get_monotonic_us();
        mPayloadSize = payloadSize;
        mpPayload.reset(new uint8_t[mPayloadSize]);
        memcpy(mpPayload.get(), pPayload.get(), mPayloadSize);

        if (0 < timestampUs) {
            mTimestampUs = timestampUs;
        }
    }

 private:
    std::unique_ptr<uint8_t[]> mpPayload;
    size_t mPayloadSize;
    int64_t mTimestampUs;
};  // class Packet

}   // namespace comm

#endif  // __PACKET_HPP__
