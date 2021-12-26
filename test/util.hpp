#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <cstdint>

#include <deque>
#include <memory>

#include "test_vectors.hpp"

#ifdef USE_RAW_POINTER
#warning "Tests will use Raw Pointers"
#else   // USE_RAW_POINTER
#warning "Tests will use Smart Pointers"
#endif  // USE_RAW_POINTER

inline bool ncompare(
    const std::unique_ptr<uint8_t[]>& arr0,
    const std::unique_ptr<uint8_t[]>& arr1,
    const size_t& size
) {

    for (size_t i = 0;i < size; i++) {
        if (arr0[i] != arr1[i]) {
            return false;
        }
    }

    return true;
}

inline bool ncompare(
    const std::unique_ptr<uint8_t[]>& arr0,
    const uint8_t* const& arr1,
    const size_t& size
) {

    for (size_t i = 0;i < size; i++) {
        if (arr0[i] != arr1[i]) {
            return false;
        }
    }

    return true;
}

inline bool test(const std::deque<std::unique_ptr<comm::Packet>>& pRxPackets) {
    const size_t EXPECTED_NUMBER_OF_PACKETS = vectors.size();
    const size_t NUMBER_OF_RX_PACKETS = pRxPackets.size();
    LOGI("Received %zu packets!\n", NUMBER_OF_RX_PACKETS);

    size_t i = 0;
    size_t packet_size;
    bool result = (EXPECTED_NUMBER_OF_PACKETS == NUMBER_OF_RX_PACKETS);
    while ((EXPECTED_NUMBER_OF_PACKETS > i) && (NUMBER_OF_RX_PACKETS > i)) {
        packet_size = pRxPackets[i]->getPayloadSize();
        LOGI("[%ld (us)] Packet %zu (%zu bytes)\n", 
            static_cast<long int>(pRxPackets[i]->getTimestampUs()), i, packet_size
        );
        if (ncompare(pRxPackets[i]->getPayload(), vectors[i], vectors_sizes[i])) {
            LOGI("  -> Matched!\n");
        } else {
            LOGI("  -> Not matched!\n");
            result = false;
        }

        i++;
    }

    return result;
}

#endif // __UTIL_HPP__
