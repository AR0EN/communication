#include "Packet.hpp"

namespace comm {

inline Packet::Packet(Packet&& other) {
    mpPayload = std::move(other.mpPayload);
    mPayloadSize = other.mPayloadSize;
    other.mPayloadSize = 0L;

    mTimestampUs = other.mTimestampUs;
    other.mTimestampUs = -1L;
}

inline Packet& Packet::operator=(Packet&& other) {
    if (this != &other) {
        mpPayload = std::move(other.mpPayload);
        mPayloadSize = other.mPayloadSize;
        other.mPayloadSize = 0L;

        mTimestampUs = other.mTimestampUs;
        other.mTimestampUs = -1L;
    }

    return *this;
}

inline Packet::Packet(
    const std::unique_ptr<uint8_t[]>& pPayload,
    const size_t& payloadSize,
    const int64_t& timestampUs) {

    mTimestampUs = get_elapsed_realtime_us();
    mPayloadSize = payloadSize;
    mpPayload.reset(new uint8_t[mPayloadSize]);
    memcpy(mpPayload.get(), pPayload.get(), mPayloadSize);

    if (0 < timestampUs) {
        mTimestampUs = timestampUs;
    }
}

inline std::unique_ptr<Packet> Packet::create(
    const std::unique_ptr<uint8_t[]>& pPayload,
    const size_t& payloadSize,
    const int64_t& timestampUs) {

    if ((pPayload) && validate_payload_size(payloadSize)) {
        return std::unique_ptr<Packet>(
            new Packet(pPayload, payloadSize, timestampUs));
    } else {
        return std::unique_ptr<Packet>(nullptr);
    }
}

inline Packet::Packet(
    const uint8_t * const& pPayload,
    const size_t& payloadSize,
    const int64_t& timestampUs) {

    mTimestampUs = get_elapsed_realtime_us();
    mPayloadSize = payloadSize;
    mpPayload.reset(new uint8_t[mPayloadSize]);
    memcpy(mpPayload.get(), pPayload, mPayloadSize);

    if (0 < timestampUs) {
        mTimestampUs = timestampUs;
    }
}

inline std::unique_ptr<Packet> Packet::create(
    const uint8_t * const& pPayload,
    const size_t& payloadSize,
    const int64_t& timestampUs) {

    if ((nullptr != pPayload) && validate_payload_size(payloadSize)) {
        return std::unique_ptr<Packet>(
            new Packet(pPayload, payloadSize, timestampUs));
    } else {
        return std::unique_ptr<Packet>(nullptr);
    }
}

}   // namespace comm
