#include <cstring>
#include <vector>

#include "Encoder.hpp"
#include "Packet.hpp"
#include "TcpClient.hpp"

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
    if (3 > argc) {
        LOGE("Usage: %s <Server Address> <Server Port>\n", argv[0]);
        return 1;
    }

    std::unique_ptr<comm::TcpClient> pTcpClient = comm::TcpClient::create(
        argv[1], static_cast<uint16_t>(atoi(argv[2]))
    );

    if (!pTcpClient) {
        LOGE("Could not create Tcp Client which connects to %s/%s!\n", argv[1], argv[2]);
        return 1;
    }

    LOGI("Connected to (%s/%s)!\n", argv[1], argv[2]);

    for (size_t i = 0; i < vectors.size(); i++) {
#ifdef USE_RAW_POINTER
        pTcpClient->send(comm::Packet::create(vectors[i], vectors_sizes[i]));
#else
        std::unique_ptr<uint8_t[]> pdata(new uint8_t[vectors_sizes[i]]);
        memcpy(pdata.get(), vectors[i], vectors_sizes[i]);
        pTcpClient->send(comm::Packet::create(pdata, vectors_sizes[i]));
#endif  // USE_RAW_POINTER
    }

    LOGI("Sent %lu packets to server, press enter to check Rx Queue ...\n", vectors.size());
    getchar();

    std::vector<std::unique_ptr<comm::Packet>> pPackets;
    if (pTcpClient->recvAll(pPackets)) {
        if (test(pPackets)) {
            LOGI("-> Passed!\n");
        } else {
            LOGI("-> Failed!\n");
        }
    } else {
        LOGE("Rx Queue is empty!\n");
    }

    LOGI("Press enter to exit ...\n");
    getchar();

    return 0;
}
