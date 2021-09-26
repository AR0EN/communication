#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#include <stdint.h>

namespace comm{

constexpr uint8_t SF = 0xF0;
constexpr uint8_t EF = 0x0F;

typedef int32_t csize_t;
constexpr csize_t MAX_PAYLOAD_SIZE = 508;   // maximum safe UDP payload

inline bool ValidatePayloadSize(csize_t payloadSize) {
    csize_t packetSize = sizeof(SF) + sizeof(csize_t) + payloadSize + sizeof(EF);
    return (0 < packetSize) && (MAX_PAYLOAD_SIZE >= packetSize);
}

}; // namespace comm

#endif // _COMMON_HPP_