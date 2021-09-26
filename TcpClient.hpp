#ifndef _TCP_CLIENT_HPP_
#define _TCP_CLIENT_HPP_

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include "common.hpp"
#include "Encoder.hpp"
#include "Message.hpp"

namespace comm {

class NetObserver {
public:
    virtual ~NetObserver() {}
    virtual void onRecv(const std::shared_ptr<NetMessage>& pMessage) = 0;
}; // class Observer

class TcpClient : public DecodingObserver, public std::enable_shared_from_this<TcpClient> {
public:
    TcpClient(const char * serverAddr, uint16_t remotePort) {
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

        mRemotePort = remotePort;

        struct sockaddr_in remoteSocketAddr;
        remoteSocketAddr.sin_family      = AF_INET;
        remoteSocketAddr.sin_addr.s_addr = inet_addr(serverAddr);
        remoteSocketAddr.sin_port = htons(mRemotePort);
        ret = connect(mSocketFd, (struct sockaddr *)&remoteSocketAddr, sizeof(remoteSocketAddr));
        if (0 != ret) {
            perror("connect() -> failed!\n");
            return;
        }

        printf("Connection has been established with %s/%d\n", serverAddr, mRemotePort);

        pThread = nullptr;
        mExitFlag = false;
    }

    ~TcpClient() {
        printf("TcpClient is finalizing ...!\n");
        stop();
    }

    void stop() {
        mExitFlag = true;

        if ((nullptr != pThread) && (pThread->joinable())) {
            pThread->join();
            pThread = nullptr;
        }

        if (0 <= mSocketFd) {
            close(mSocketFd);
            mSocketFd = -1;
        }

        printf("TcpClient has been finalized!\n");
    }

    csize_t send(std::shared_ptr<Message> pMessage);

    bool start();
    bool subscribe(const std::shared_ptr<NetObserver>& pObserver);
    // void unsubscribe() = 0;

    // DecodingObserver implementation
    void onComplete(const std::shared_ptr<uint8_t>& pData, int size) {
        if (pData) {
            uint8_t * tmp = pData.get();

            std::shared_ptr<NetMessage> pMessage(new NetMessage());
            pMessage->deserialize(tmp, size);

            notify(pMessage);
        }
    }

private:
    void notify (const std::shared_ptr<NetMessage>& pMessage) {
        for (auto pObserver : mObservers) {
            pObserver->onRecv(pMessage);
        }
    }

    void run();

    int mSocketFd;
    uint16_t mRemotePort;

    uint8_t mRxBuffer[MAX_PAYLOAD_SIZE << 1];

    std::shared_ptr<Decoder> pDecoder;
    std::vector<std::shared_ptr<NetObserver>> mObservers;

    std::unique_ptr<std::thread> pThread;
    std::atomic<bool> mExitFlag;

    static constexpr int RX_TIMEOUT_S = 1;
}; // class Peer

}; // namespace comm

#endif // _TCP_CLIENT_HPP_
