#ifndef __UDPPEER_HPP__
#define __UDPPEER_HPP__

#include <arpa/inet.h>
#include <cstdint>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include <memory>
#include <thread>

#include "common.hpp"
#include "Endpoint.hpp"
#include "Packet.hpp"

namespace comm {

class UdpPeer : public EndPoint {
 public:
    virtual ~UdpPeer() {
        stop();
        LOGI("[%s][%d] Finalized!\n", __func__, __LINE__);
    }

    static std::unique_ptr<UdpPeer> create(
        const uint16_t& localPort, const std::string& peerAddress, const uint16_t& peerPort
    ) {
        std::unique_ptr<UdpPeer> udpPeer;

        if (0 == localPort) {
            LOGE("[%s][%d] Peer 's Port must be a positive value!\n", __func__, __LINE__);
            return udpPeer;
        }

        int socketFd = socket(AF_INET, SOCK_DGRAM, 0);
        if (0 > socketFd) {
            perror("Could not create UDP socket!\n");
            return udpPeer;
        }

        int enable = 1;
        if (0 > setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
            perror("Failed to enable SO_REUSEADDR!\n");
            close(socketFd);
            return udpPeer;
        }

        struct timeval timeout;
        timeout.tv_sec = RX_TIMEOUT_S;
        timeout.tv_usec = 0;

        if (0 > setsockopt (socketFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))) {
            perror("Failed to configure SO_RCVTIMEO!\n");
            close(socketFd);
            return udpPeer;
        }

        struct sockaddr_in localSocketAddr;
        localSocketAddr.sin_family      = AF_INET;
        localSocketAddr.sin_addr.s_addr = INADDR_ANY;
        localSocketAddr.sin_port        = htons(localPort);
        int ret = bind(socketFd, (struct sockaddr *)&localSocketAddr, sizeof(localSocketAddr));
        if (0 > ret) {
            perror("Failed to bind socket!\n");
            close(socketFd);
            return nullptr;
        }

        LOGI("[%s][%d] Bound at port %u\n", __func__, __LINE__, localPort);
        return std::unique_ptr<UdpPeer>(new UdpPeer(socketFd, localPort, peerAddress, peerPort));
    }

    void stop() {
        mExitFlag = true;

        if ((mpRxThread) && (mpRxThread->joinable())) {
            mpRxThread->join();
            mpRxThread.reset();
        }

        if ((mpTxThread) && (mpTxThread->joinable())) {
            mpTxThread->join();
            mpTxThread.reset();
        }

        if (0 <= mSocketFd) {
            close(mSocketFd);
            mSocketFd = -1;
        }
    }

    bool setDestination(const std::string& address, const uint16_t& port) {
        if (address.empty() || (0 == port)) {
            LOGE("[%s][%d] Invalid peer information!\n", __func__, __LINE__);
            return false;
        }

        std::lock_guard<std::mutex> lock(mPeerMutex);
        mPeerAddress = address;
        mPeerPort = port;

        return true;
    }

    bool checkRxPipe() override {
        return (0 < mSocketFd);
    }

    bool checkTxPipe() override {
        return (!mPeerAddress.empty()) && (0 < mPeerPort);
    }

 protected:
    UdpPeer(
        const int& socketFd, const uint16_t& localPort,
        const std::string& peerAddress, const uint16_t& peerPort
    ) {
        mSocketFd = socketFd;
        mLocalPort = localPort;
        mPeerAddress = peerAddress;
        mPeerPort = peerPort;
        mExitFlag = false;
        mpRxThread.reset(new std::thread(&comm::UdpPeer::runRx, this));
        mpTxThread.reset(new std::thread(&comm::UdpPeer::runTx, this));
    }

    ssize_t lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) override {
        socklen_t remoteAddressSize;
        struct sockaddr_in remoteSocketAddr;
        remoteAddressSize = (socklen_t)sizeof(remoteSocketAddr);

        ssize_t ret = recvfrom(
                        mSocketFd, pBuffer.get(), limit, 0,
                        reinterpret_cast<struct sockaddr *>(&remoteSocketAddr), &remoteAddressSize
                    );

        if (0 > ret) {
            if (EWOULDBLOCK == errno) {
                ret = 0;
            } else {
                perror("");
            }
        } else {
            LOGD("[%s][%d] Received %ld bytes\n", __func__, __LINE__, ret);
        }

        return ret;
    }

    ssize_t lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) override {
        std::lock_guard<std::mutex> lock(mPeerMutex);

        // Destination address
        struct sockaddr_in peerAddr;
        peerAddr.sin_family      = AF_INET;
        peerAddr.sin_port        = htons(mPeerPort);
        peerAddr.sin_addr.s_addr = inet_addr(mPeerAddress.c_str());

        // Send data over UDP
        ssize_t ret = 0LL;
        for (int i = 0; i < TX_RETRY_COUNT; i++) {
            ret = sendto(
                            mSocketFd, pData.get(), size, 0,
                            reinterpret_cast<struct sockaddr *>(&peerAddr), sizeof(peerAddr)
                        );

            if (0 > ret) {
                if (EWOULDBLOCK != errno) {
                    perror("");
                    break;
                } else {
                    // Ignore & retry
                }
            } else {
                LOGD("[%s][%d] Transmitted %ld bytes\n", __func__, __LINE__, ret);
                break;
            }
        }

        return ret;
    }

 private:
    void runRx() {
        while(!mExitFlag) {
            if (!proceedRx()) {
                LOGE("[%s][%d]\n", __func__, __LINE__);
                break;
            }
        }
    }

    void runTx() {
        while(!mExitFlag) {
            if (!proceedTx()) {
                LOGE("[%s][%d]\n", __func__, __LINE__);
                break;
            }
        }
    }

    // Local
    int mSocketFd;
    uint16_t mLocalPort;

    // Peer
    std::string mPeerAddress;
    uint16_t mPeerPort;
    std::mutex mPeerMutex;

    std::unique_ptr<std::thread> mpRxThread;
    std::unique_ptr<std::thread> mpTxThread;
    std::atomic<bool> mExitFlag;
};  // class Peer

}   // namespace comm

#endif // __UDPPEER_HPP__
