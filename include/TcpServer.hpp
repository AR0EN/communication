#ifndef _TCP_SERVER_HPP_
#define _TCP_SERVER_HPP_

#include "common.hpp"
#include "Encoder.hpp"
#include "Message.hpp"

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

namespace comm {

class TcpServer : public DecodingObserver, public std::enable_shared_from_this<TcpServer> {
public:
    TcpServer(uint16_t rxPort) {
        mRemoteSocketFd = -1;

        mSocketFd = socket(AF_INET, SOCK_STREAM, 0);
        if (0 > mSocketFd) {
            perror("Could not create TCP socket!\n");
            mExitFlag = true;
            return;
        }

        int ret;

        int enable = 1;
        ret = setsockopt(mSocketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
        if (0 > ret) {
            perror("setsockopt(SO_REUSEADDR) -> false!\n");
            mExitFlag = true;
            return;
        }

        struct timeval timeout;
        timeout.tv_sec = RX_TIMEOUT_S;
        timeout.tv_usec = 0;
        ret = setsockopt (mSocketFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        if (0 > ret) {
            perror("setsockopt(SO_RCVTIMEO) -> failed!\n");
            mExitFlag = true;
            return;
        }

        mRxPort = rxPort;

        struct sockaddr_in localSocketAddr;
        localSocketAddr.sin_family      = AF_INET;
        localSocketAddr.sin_addr.s_addr = INADDR_ANY;
        localSocketAddr.sin_port = htons(mRxPort);
        ret = bind(mSocketFd, (struct sockaddr *)&localSocketAddr, sizeof(localSocketAddr));
        if (0 > ret) {
            perror("bind() -> failed!\n");
            return;
        }

        printf("Bound at port %d\n", mRxPort);

        ret = listen(mSocketFd, 3);
        if (0 != ret) {
            perror("listen() -> failed!\n");
            return;
        }

        mExitFlag = false;
    }

    ~TcpServer() {
        printf("TcpServer is finalizing ...!\n");
        stop();
    }

    void stop() {
        mExitFlag = true;

        if ((pThread) && (pThread->joinable())) {
            pThread->join();
            pThread.reset();
        }

        if (0 <= mRemoteSocketFd) {
            close(mRemoteSocketFd);
            mRemoteSocketFd = -1;
        }

        if (0 <= mSocketFd) {
            close(mSocketFd);
            mSocketFd = -1;
        }

        printf("TcpServer has been finalized!\n");
    }

    size_t send(IMessage& message);

    bool start();
    bool subscribe(const std::shared_ptr<IObserver>& pObserver);
    // void unsubscribe() = 0;

    // DecodingObserver implementation
    void onComplete(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) {
        if (pData) {
            std::unique_ptr<Message> pMessage(new Message());
            pMessage->deserialize(pData, size);

            notify(pMessage);
        }
    }

private:
    void notify (const std::unique_ptr<Message>& pMessage) {
        for (auto pObserver : mObservers) {
            pObserver->onRecv(pMessage);
        }
    }

    void run();
    void receive();

    int mSocketFd;
    std::atomic<int> mRemoteSocketFd;
    uint16_t mRxPort;

    uint8_t mRxBuffer[MAX_PAYLOAD_SIZE << 1];

    std::unique_ptr<Decoder> pDecoder;
    std::vector<std::shared_ptr<IObserver>> mObservers;

    std::unique_ptr<std::thread> pThread;
    std::atomic<bool> mExitFlag;

    static constexpr int RX_TIMEOUT_S = 1;
}; // class Peer

}; // namespace comm

#endif // _TCP_SERVER_HPP_
