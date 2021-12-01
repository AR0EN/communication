#ifndef _TCP_CLIENT_HPP_
#define _TCP_CLIENT_HPP_

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

        LOGI("Connection has been established with %s/%d\n", serverAddr, mRemotePort);

        mExitFlag = false;
    }

    ~TcpClient() {
        LOGI("TcpClient is finalizing ...!\n");
        stop();
    }

    void stop() {
        mExitFlag = true;

        if ((pThread) && (pThread->joinable())) {
            pThread->join();
            pThread.reset();
        }

        if (0 <= mSocketFd) {
            close(mSocketFd);
            mSocketFd = -1;
        }

        LOGI("TcpClient has been finalized!\n");
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

    int mSocketFd;
    uint16_t mRemotePort;

    uint8_t mRxBuffer[MAX_PAYLOAD_SIZE << 1];

    std::unique_ptr<Decoder> pDecoder;
    std::vector<std::shared_ptr<IObserver>> mObservers;

    std::unique_ptr<std::thread> pThread;
    std::atomic<bool> mExitFlag;

    static constexpr int RX_TIMEOUT_S = 1;
}; // class Peer

}; // namespace comm

#endif // _TCP_CLIENT_HPP_
