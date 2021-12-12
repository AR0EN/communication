#ifndef _TEST_VECTORS_HPP_
#define _TEST_VECTORS_HPP_

#include <cstdint>

#include <vector>

#ifdef USE_RAW_POINTER
#warning "Tests will use Raw Pointers"
#else
#warning "Tests will use Smart Pointers"
#endif

extern std::vector<const uint8_t *> vectors;
extern std::vector<size_t> vectors_sizes;

#endif  // _TEST_VECTORS_HPP_
