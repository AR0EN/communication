#include <cstring>
#include <deque>

#include "common.hpp"
#include "Encoder.hpp"
#include "Packet.hpp"
#include "TcpClient.hpp"

#include "test_vectors.hpp"
#include "util.hpp"

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
        LOGI("[%lld (us)] Sent packet %zu (%zu bytes)\n", 
            static_cast<long long int>(get_elapsed_realtime_us()),
            i, vectors_sizes[i]
        );
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

    std::deque<std::unique_ptr<comm::Packet>> pPackets;
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
