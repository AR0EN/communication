#include <cstring>
#include <deque>

#include "common.hpp"
#include "Encoder.hpp"
#include "Packet.hpp"
#include "TcpClient.hpp"

#include "test_vectors.hpp"
#include "util.hpp"

#define EP_NAME "TcpClient"

int main(int argc, char ** argv) {
    if (3 > argc) {
        LOGE("Usage: %s <Server Address> <Server Port>\n", argv[0]);
        return 1;
    }

    std::unique_ptr<comm::P2P_Endpoint> pEndpoint = comm::TcpClient::create(
        argv[1], static_cast<uint16_t>(atoi(argv[2]))
    );

    if (!pEndpoint) {
        LOGE("Could not create %s which listens at port %s!\n", EP_NAME, argv[1]);
        return 1;
    }

    LOGI("%s is ready, waiting for peer ...\n", EP_NAME);

    auto t0 = get_monotonic_clock();
    while (!pEndpoint->isPeerConnected()) {
        if (
            std::chrono::seconds(10) <
            std::chrono::duration_cast<std::chrono::seconds>(get_monotonic_clock() - t0)
        ) {
            LOGE("Timeout!\n");
            return 1;
        }
    }

    LOGI("Connected to peer, press enter to sent data to peer ...\n");
    getchar();

    for (size_t i = 0; i < vectors.size(); i++) {
        LOGI("[%lld (us)] Sending packet %zu (%zu bytes) ...\n", 
            static_cast<long long int>(get_elapsed_realtime_us()),
            i, vectors_sizes[i]
        );
#ifdef USE_RAW_POINTER
        pEndpoint->send(comm::Packet::create(vectors[i], vectors_sizes[i]));
#else
        std::unique_ptr<uint8_t[]> pdata(new uint8_t[vectors_sizes[i]]);
        memcpy(pdata.get(), vectors[i], vectors_sizes[i]);
        pEndpoint->send(comm::Packet::create(pdata, vectors_sizes[i]));
#endif  // USE_RAW_POINTER
    }

    LOGI("Press enter to check Rx Queue ...\n");
    getchar();

    std::deque<std::unique_ptr<comm::Packet>> pPackets;
    if (pEndpoint->recvAll(pPackets)) {
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
