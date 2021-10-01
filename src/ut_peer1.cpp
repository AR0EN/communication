#include <stdio.h>
#include <stdlib.h>

#include <atomic>
#include <memory>

#include "test_vectors.hpp"

#include "Encoder.hpp"
#include "Message.hpp"
#include "UdpPeer.hpp"

std::atomic<bool> start;

class NetObserverImpl : public comm::NetObserver {
public:
    NetObserverImpl() {}
    ~NetObserverImpl() {}
    void onRecv(const std::shared_ptr<comm::NetMessage>& pMessage) {
        if (nullptr != pMessage) {
            pMessage->dump();
            start = true;
        }
    }
};

int main() {
    start = false;
    std::shared_ptr<comm::UdpPeer> pPeer(new comm::UdpPeer(3188));
    std::shared_ptr<comm::NetObserver> pObserver(new NetObserverImpl());
    pPeer->subscribe(pObserver);
    pPeer->start();

    for (int i = 0; i < vectors.size(); i++) {
        int index = vectors.size() - i - 1;
        std::shared_ptr<comm::NetMessage> pMessage(new comm::NetMessage());
        pMessage->update(vectors_identifiers[index], vectors_sizes[index], vectors[index]);
        pPeer->send("127.0.0.1", 9696, pMessage);
    }

    printf("Press any key to exit!\n");
    getchar();

    pPeer->stop();

    return 0;
}
