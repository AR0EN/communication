#include "ObserverImpl.hpp"
#include "test_vectors.hpp"

#include "Encoder.hpp"
#include "Message.hpp"
#include "TcpServer.hpp"

#include <memory>

#include <stdio.h>
#include <stdlib.h>

// ./tcp_server <local port>
int main(int argc, char ** argv) {
    if (2 > argc) {
        printf("Missing arguments!\n");
        printf("Usage:\n%s <local port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    std::shared_ptr<comm::TcpServer> pTcpServer(new comm::TcpServer(atoi(argv[1])));
    std::shared_ptr<comm::IObserver> pObserver = std::make_shared<ObserverImpl>();
    pTcpServer->subscribe(pObserver);
    pTcpServer->start();

    printf("Press enter to continue!\n");
    getchar();

    comm::Message message;
    for (int i = 0; i < vectors.size(); i++) {
        int index = vectors.size() - i - 1;
        message.update(vectors[index], vectors_sizes[index]);
        pTcpServer->send(message);
    }

    printf("Press enter to exit!\n");
    getchar();

    pTcpServer->stop();

    return 0;
}
