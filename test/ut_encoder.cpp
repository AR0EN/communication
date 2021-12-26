#include <cstring>
#include <deque>

#include "Encoder.hpp"
#include "Packet.hpp"

#include "test_vectors.hpp"
#include "util.hpp"

bool test(const std::unique_ptr<uint8_t[]>& pdata, const size_t& size, const size_t& max_chunk_size = 128) {
    static uint16_t tid = 0;
    bool result;

    comm::Decoder decoder;

    // Encoding
    std::unique_ptr<uint8_t[]> pencoded_data;
    size_t encoded_size = 0ULL;

    result = comm::encode(pdata, size, tid++, pencoded_data, encoded_size);
    LOGI("  * Step 1: encoded %zu bytes to %zu bytes\n",
        size, encoded_size
    );

    if (!result) {
        return result;
    }

    // Decoding
    LOGI("  * Step 2:\n");
    const uint8_t * internal_pointer = pencoded_data.get();
    std::unique_ptr<uint8_t[]> ptmp;
    size_t chunk_size;

    while (0 < encoded_size) {
        chunk_size = (max_chunk_size < encoded_size)? max_chunk_size:encoded_size;
        encoded_size -= chunk_size;
        LOGD("Feed %zu bytes -> %zu bytes left!\n", chunk_size, encoded_size);
        ptmp.reset(new uint8_t[chunk_size]);
        memcpy(ptmp.get(), internal_pointer, chunk_size);
        decoder.feed(ptmp, chunk_size);

        internal_pointer += chunk_size;
    }

    std::deque<std::unique_ptr<comm::Packet>> pdecoded_packets;
    if (decoder.dequeue(pdecoded_packets)) {
        for (auto& ppacket : pdecoded_packets) {
            result &= size == ppacket->getPayloadSize();
            if (result) {
                result &= ncompare(ppacket->getPayload(), pdata, size);
            }
            LOGI("  -> Decoded %zu bytes\n",
                ppacket->getPayloadSize()
            );
        }
    } else {
        result = false;
        LOGI("  -> No packet has been decoded!\n");
    }

    return result;
}

int main() {
    for (size_t i = 0; i < vectors.size(); i++) {
        std::unique_ptr<uint8_t[]> pdata(new uint8_t[vectors_sizes[i]]);
        memcpy(pdata.get(), vectors[i], vectors_sizes[i]);
        LOGI("Test case %02zu:\n", i);
        LOGI("-> %s\n\n", test(pdata, vectors_sizes[i]) ? "Passed" : "Failed");
    }

    return 0;
}
