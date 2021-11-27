#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#include <chrono>
#include <cstdint>

namespace comm{

// Frame Structure
// 0xF0 (uint8_t) | Size of Payload (uint32_t LE) | Payload | 0x0F (uint8_t)
constexpr uint8_t SF                   = 0xF0;
constexpr size_t SF_SIZE               = 1;

constexpr uint8_t EF                   = 0x0F;
constexpr size_t EF_SIZE               = 1;

constexpr size_t SIZE_OF_PAYLOAD_SIZE  = 4;

constexpr size_t MAX_PAYLOAD_SIZE      = 1024;  // Avoid network fragmentation

inline bool validate_payload_size(const size_t& payload_size) {
    return (MAX_PAYLOAD_SIZE >= (SF_SIZE + SIZE_OF_PAYLOAD_SIZE + payload_size + EF_SIZE));
}

inline int64_t get_monotonic_us() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

}; // namespace comm

#endif // _COMMON_HPP_