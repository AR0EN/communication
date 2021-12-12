#ifndef __PACKET_HPP__
#define __PACKET_HPP__

#include <cstdint>
#include <cstring>
#include <memory>

#include "common.hpp"

namespace comm {

class Packet {
 public:
    Packet(Packet&& other);

    Packet& operator=(Packet&& other);

    Packet() = delete;
    Packet(const Packet&) = delete;
    Packet& operator=(const Packet&) = delete;

    virtual ~Packet() {}

    static std::unique_ptr<Packet> create(
        const std::unique_ptr<uint8_t[]>& pPayload,
        const size_t& payloadSize,
        const int64_t& timestampUs=-1
    );

    static std::unique_ptr<Packet> create(
        const uint8_t * const& pPayload,
        const size_t& payloadSize,
        const int64_t& timestampUs=-1
    );

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
        const int64_t& timestampUs
    );

    Packet(
        const uint8_t * const& pPayload,
        const size_t& payloadSize,
        const int64_t& timestampUs
    );

 private:
    std::unique_ptr<uint8_t[]> mpPayload;
    size_t mPayloadSize;
    int64_t mTimestampUs;
};  // class Packet

}   // namespace comm

#include "inline/Packet.inl"

#endif  // __PACKET_HPP__
