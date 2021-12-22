#include <cstring>

#include <chrono>
#include <deque>
#include <thread>

#include "common.hpp"
#include "Encoder.hpp"
#include "Packet.hpp"
#include "UdpPeer.hpp"

#include "test_vectors.hpp"
#include "util.hpp"

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

    LOGI("Press enter to send data to %s/%u ...\n", argv[2], static_cast<uint16_t>(atoi(argv[3])));
    getchar();

    for (size_t i = 0; i < vectors.size(); i++) {
        LOGI("[%lld (us)] Sent packet %zu (%zu bytes)\n", 
            static_cast<long long int>(get_elapsed_realtime_us()),
            i, vectors_sizes[i]
        );
#ifdef USE_RAW_POINTER
        pUdpPeer->send(comm::Packet::create(vectors[i], vectors_sizes[i]));
#else
        std::unique_ptr<uint8_t[]> pdata(new uint8_t[vectors_sizes[i]]);
        memcpy(pdata.get(), vectors[i], vectors_sizes[i]);
        pUdpPeer->send(comm::Packet::create(pdata, vectors_sizes[i]));
#endif  // USE_RAW_POINTER
    }

    LOGI("Press enter to check Rx Queue ...\n");
    getchar();

    std::deque<std::unique_ptr<comm::Packet>> pPackets;
    if (pUdpPeer->recvAll(pPackets)) {
        if (test(pPackets)) {
            LOGI("-> Passed!\n");
        } else {
            LOGI("-> Failed!\n");
        }
    } else {
        LOGI("Rx Queue is empty!\n");
    }

    LOGI("Press enter to exit ...\n");
    getchar();

    return 0;
}
