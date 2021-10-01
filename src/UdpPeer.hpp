#ifndef _UDP_PEER_HPP_
#define _UDP_PEER_HPP_

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
#include "Observer.hpp"

namespace comm {

class UdpPeer : public DecodingObserver, public std::enable_shared_from_this<UdpPeer> {
public:
    UdpPeer(uint16_t rxPort) {
        mSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
        if (0 > mSocketFd) {
            perror("Could not create UDP socket!\n");
            mExitFlag = true;
            return;
        }

        int enable = 1;
        if (0 > setsockopt(mSocketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
            perror("setsockopt(SO_REUSEADDR) -> false!\n");
            mExitFlag = true;
            return;
        }

        struct timeval timeout;
        timeout.tv_sec = RX_TIMEOUT_S;
        timeout.tv_usec = 0;

        if (0 > setsockopt (mSocketFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))) {
            perror("setsockopt(SO_RCVTIMEO) -> failed!\n");
            mExitFlag = true;
            return;
        }

        mRxPort = rxPort;

        struct sockaddr_in localSocketAddr;
        localSocketAddr.sin_family      = AF_INET;
        localSocketAddr.sin_addr.s_addr = INADDR_ANY;
        localSocketAddr.sin_port = htons(mRxPort);
        int ret = bind(mSocketFd, (struct sockaddr *)&localSocketAddr, sizeof(localSocketAddr));
        if (0 > ret) {
            perror("bind() -> failed!\n");
            return;
        }

        printf("Bound at port %d\n", mRxPort);

        pThread = nullptr;
        mExitFlag = false;
    }

    ~UdpPeer() {
        printf("Peer is finalizing ...!\n");
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

        printf("Finalized!\n");
    }

    csize_t send(const char * ipAddress, uint16_t port, std::shared_ptr<Message> pMessage);

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
    uint16_t mRxPort;

    uint8_t mRxBuffer[MAX_PAYLOAD_SIZE << 1];

    std::shared_ptr<Decoder> pDecoder;
    std::vector<std::shared_ptr<NetObserver>> mObservers;

    std::unique_ptr<std::thread> pThread;
    std::atomic<bool> mExitFlag;

    static constexpr int RX_TIMEOUT_S = 1;
}; // class Peer

}; // namespace comm

#endif // _UDP_PEER_HPP_
