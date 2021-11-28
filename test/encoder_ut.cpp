#include <cstdio>
#include <cstring>
#include <vector>

#include "Encoder.hpp"
#include "Packet.hpp"

#include "test_vectors.hpp"

bool compare(
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

bool test(const std::unique_ptr<uint8_t[]>& pdata, const size_t& size, const size_t& max_chunk_size = 128) {
    bool result;

    comm::Decoder decoder;

    // Encoding
    std::unique_ptr<uint8_t[]> pencoded_data;
    size_t encoded_size;

    result = comm::encode(pdata, size, pencoded_data, encoded_size);
    printf("  * Step 1: encoded %lu bytes to %lu bytes\n",
        static_cast<uint64_t>(size), static_cast<uint64_t>(encoded_size)
    );

    if (!result) {
        return result;
    }

    // Decoding
    printf("  * Step 2:\n");
    const uint8_t * internal_pointer = pencoded_data.get();
    std::unique_ptr<uint8_t[]> ptmp;
    size_t chunk_size;

    while (0 < encoded_size) {
        chunk_size = (max_chunk_size < encoded_size)? max_chunk_size:encoded_size;
        encoded_size -= chunk_size;
#ifdef DEBUG
        printf("Feed %lu bytes -> %lu bytes left!\n",
            static_cast<uint64_t>(chunk_size), static_cast<uint64_t>(encoded_size)
        );
#endif  // DEBUG
        ptmp.reset(new uint8_t[chunk_size]);
        memcpy(ptmp.get(), internal_pointer, chunk_size);
        decoder.feed(ptmp, chunk_size);

        internal_pointer += chunk_size;
    }

    std::vector<std::unique_ptr<comm::Packet>> decoded_packets;
    if (decoder.dequeue(decoded_packets)) {
        for (auto& ppacket : decoded_packets) {
            result &= size == ppacket->getPayloadSize();
            if (result) {
                result &= compare(ppacket->getPayload(), pdata, size);
            }
            printf("  -> Decoded %lu bytes\n",
                static_cast<uint64_t>(ppacket->getPayloadSize())
            );
        }
    } else {
        result = false;
        printf("  -> No packet has been decoded!\n");
    }

    return result;
}

int main() {
    for (size_t i = 0; i < vectors.size(); i++) {
        std::unique_ptr<uint8_t[]> pdata(new uint8_t[vectors_sizes[i]]);
        memcpy(pdata.get(), vectors[i], vectors_sizes[i]);
        printf("Test case %02lu:\n", static_cast<uint64_t>(i));
        printf("-> %s\n\n", test(pdata, vectors_sizes[i]) ? "Passed" : "Failed");
    }

    return 0;
}
