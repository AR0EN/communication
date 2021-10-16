#include "ObserverImpl.hpp"
#include "test_vectors.hpp"

#include "Encoder.hpp"
#include "Message.hpp"
#include "TcpClient.hpp"

#include <memory>

#include <stdio.h>
#include <stdlib.h>

// ./tcp_client <remote address> <remote port>
int main(int argc, char ** argv) {
    if (3 > argc) {
        printf("Missing arguments!\n");
        printf("Usage:\n%s <remote address> <remote port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    std::shared_ptr<comm::TcpClient> pTcpClient(new comm::TcpClient(argv[1], atoi(argv[2])));
    std::shared_ptr<comm::IObserver> pObserver = std::make_shared<ObserverImpl>();
    pTcpClient->subscribe(pObserver);
    pTcpClient->start();

    comm::Message message;
    for (int i = 0; i < vectors.size(); i++) {
        int index = vectors.size() - i - 1;
        message.update(vectors[index], vectors_sizes[index]);
        pTcpClient->send(message);
    }

    printf("Press enter to exit!\n");
    getchar();

    pTcpClient->stop();

    return 0;
}
