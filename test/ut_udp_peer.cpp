#include <cstring>
#include <vector>

#include "Encoder.hpp"
#include "Packet.hpp"
#include "UdpPeer.hpp"

#include "test_vectors.hpp"
#include "util.hpp"

bool test(const std::vector<std::unique_ptr<comm::Packet>>& pRxPackets) {
    int i = 0;
    for (auto& pPacket : pRxPackets) {
        size_t packet_size = pPacket->getPayloadSize();
        LOGI("Packet %d (%lu bytes)\n", i, static_cast<uint64_t>(packet_size));
        if (vectors_sizes[i] == packet_size) {
            std::unique_ptr<uint8_t[]> pTestData (new uint8_t[vectors_sizes[i]]);
            memcpy(pTestData.get(), vectors[i], vectors_sizes[i]);
            if (compare(pTestData, pPacket->getPayload(), vectors_sizes[i])) {
                LOGI("  -> Matched!\n");
            } else {
                LOGI("  -> Data is not matched!\n");
                return false;
            }
        } else {
            LOGI("  -> Packet length is not matched (expected: %lu bytes)!\n", vectors_sizes[i]);
            return false;
        }

        i++;
    }

    return true;
}

int main(int argc, char ** argv) {
    if (4 > argc) {
        LOGI("Usage: %s <Local Port> <Peer Address> <Peer Port>\n", argv[0]);
        return 1;
    }

    std::unique_ptr<comm::UdpPeer> pUdpPeer = comm::UdpPeer::create(
        static_cast<uint16_t>(atoi(argv[1])), argv[2], static_cast<uint16_t>(atoi(argv[3]))
    );

    if (!pUdpPeer) {
        LOGI("Could not create UdpPeer which listens at port %s!\n", argv[1]);
        return 1;
    }

    LOGI("Press any key to continue ...\n");
    getchar();

    for (size_t i = 0; i < vectors.size(); i++) {
        std::unique_ptr<uint8_t[]> pdata(new uint8_t[vectors_sizes[i]]);
        memcpy(pdata.get(), vectors[i], vectors_sizes[i]);
        pUdpPeer->send(comm::Packet::create(pdata, vectors_sizes[i]));
    }

    LOGI("Press any key to check Rx queue ...\n");
    getchar();

    std::vector<std::unique_ptr<comm::Packet>> pPackets;
    if (pUdpPeer->recvAll(pPackets)) {
        if (test(pPackets)) {
            LOGI("-> Passed!\n");
        } else {
            LOGI("-> Failed!\n");
        }
    } else {
        LOGI("Rx queue is empty!\n");
    }

    LOGI("Press any key to exit ...\n");
    getchar();

    return 0;
}
