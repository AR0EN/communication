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

inline bool compare(
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

inline bool compare(
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
    bool result = (vectors.size() == pRxPackets.size());

    size_t i = 0;
    size_t number_of_tests = (vectors.size() < pRxPackets.size()) ? vectors.size() : pRxPackets.size();
    
    for (auto& pPacket : pRxPackets) {
        size_t packet_size = pPacket->getPayloadSize();
        LOGI("Packet %zu (%zu bytes)\n", i, packet_size);
        if (vectors_sizes[i] == packet_size) {
            if (compare(pPacket->getPayload(), vectors[i], vectors_sizes[i])) {
                LOGI("  -> Matched!\n");
            } else {
                LOGI("  -> Data is not matched!\n");
                result = false;
            }
        } else {
            LOGI("  -> Packet length is not matched (expected: %zu bytes)!\n", vectors_sizes[i]);
            result = false;
        }

        if (number_of_tests <= ++i) {
            break;
        }
    }

    return result;
}

#endif // __UTIL_HPP__
