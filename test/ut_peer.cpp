#include "ObserverImpl.hpp"
#include "test_vectors.hpp"

#include "Encoder.hpp"
#include "Message.hpp"
#include "UdpPeer.hpp"

#include <memory>

#include <stdio.h>
#include <stdlib.h>


// ./peer <local port> <remote address> <remote port>
int main(int argc, char ** argv) {
    if (4 > argc) {
        printf("Missing arguments!\n");
        printf("Usage:\n%s <local port> <remote address> <remote port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    std::shared_ptr<comm::UdpPeer> pPeer(new comm::UdpPeer(atoi(argv[1])));
    std::shared_ptr<comm::IObserver> pObserver = std::make_shared<ObserverImpl>();
    pPeer->subscribe(pObserver);
    pPeer->start();

    printf("Press enter to continue!\n");
    getchar();

    comm::Message message;
    for (int i = 0; i < vectors.size(); i++) {
        message.update(vectors[i], vectors_sizes[i]);
        pPeer->send(argv[2], atoi(argv[3]), message);
    }

    printf("Press enter to exit!\n");
    getchar();

    pPeer->stop();

    return 0;
}
