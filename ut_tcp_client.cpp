#include <stdio.h>
#include <stdlib.h>

#include <atomic>
#include <memory>

#include "test_vectors.hpp"

#include "Encoder.hpp"
#include "Message.hpp"
#include "TcpClient.hpp"

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
    std::shared_ptr<comm::TcpClient> pTcpClient(new comm::TcpClient("127.0.0.1", 9696));
    std::shared_ptr<comm::NetObserver> pObserver(new NetObserverImpl());
    pTcpClient->subscribe(pObserver);
    pTcpClient->start();

    for (int i = 0; i < vectors.size(); i++) {
        int index = vectors.size() - i - 1;
        std::shared_ptr<comm::NetMessage> pMessage(new comm::NetMessage());
        pMessage->update(vectors_identifiers[index], vectors_sizes[index], vectors[index]);
        pTcpClient->send(pMessage);
    }

    printf("Press any key to exit!\n");
    getchar();

    pTcpClient->stop();

    return 0;
}
