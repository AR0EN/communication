#include <cstring>
#include <deque>

#include "common.hpp"
#include "Encoder.hpp"
#include "Packet.hpp"
#include "TcpServer.hpp"

#include "test_vectors.hpp"
#include "util.hpp"

int main(int argc, char ** argv) {
    if (2 > argc) {
        LOGE("Usage: %s <Local Port>\n", argv[0]);
        return 1;
    }

    std::unique_ptr<comm::TcpServer> pTcpServer = comm::TcpServer::create(
        static_cast<uint16_t>(atoi(argv[1]))
    );

    if (!pTcpServer) {
        LOGE("Could not create Tcp Server which listens at port %s!\n", argv[1]);
        return 1;
    }

    LOGI("Tcp Server is ready, waiting for client ...\n");

    auto t0 = get_monotonic_clock();
    while (!pTcpServer->isClientConnected()) {
        if (
            std::chrono::seconds(10) <
            std::chrono::duration_cast<std::chrono::seconds>(get_monotonic_clock() - t0)
        ) {
            LOGE("Timeout!\n");
            return 1;
        }
    }

    LOGI("Connected to client, please enter to check Rx Queue ...\n");
    getchar();

    std::deque<std::unique_ptr<comm::Packet>> pPackets;
    if (pTcpServer->recvAll(pPackets)) {
        if (test(pPackets)) {
            LOGI("-> Passed!\n");
        } else {
            LOGI("-> Failed!\n");
        }
    } else {
        LOGE("Rx Queue is empty!\n");
    }

    LOGI("Press enter to sent data to client ...\n");
    getchar();

    for (size_t i = 0; i < vectors.size(); i++) {
        LOGI("[%lld (us)] Sent packet %zu (%zu bytes)\n", 
            static_cast<long long int>(get_elapsed_realtime_us()),
            i, vectors_sizes[i]
        );
#ifdef USE_RAW_POINTER
        pTcpServer->send(comm::Packet::create(vectors[i], vectors_sizes[i]));
#else
        std::unique_ptr<uint8_t[]> pdata(new uint8_t[vectors_sizes[i]]);
        memcpy(pdata.get(), vectors[i], vectors_sizes[i]);
        pTcpServer->send(comm::Packet::create(pdata, vectors_sizes[i]));
#endif  // USE_RAW_POINTER
    }

    LOGI("Press enter to exit ...\n");
    getchar();

    return 0;
}
