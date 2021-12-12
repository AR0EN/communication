#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <cstdint>
#include <chrono>
#include <memory>

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

inline std::chrono::time_point<std::chrono::steady_clock> get_monotonic_clock() {
    return std::chrono::steady_clock::now();
}

#endif // __UTIL_HPP__
