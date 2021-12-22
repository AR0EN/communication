#ifndef __UDPPEER_HPP__
#define __UDPPEER_HPP__

#include <arpa/inet.h>
#include <cstdint>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include <memory>
#include <mutex>
#include <thread>

#include "common.hpp"
#include "P2P_Endpoint.hpp"
#include "Packet.hpp"

namespace comm {

class UdpPeer : public P2P_Endpoint {
 public:
    bool setDestination(const std::string& address, const uint16_t& port);

    void close() override;

    virtual ~UdpPeer();
    static std::unique_ptr<UdpPeer> create(
        const uint16_t& localPort, const std::string& peerAddress, const uint16_t& peerPort
    );

 protected:
    UdpPeer(
        const int& socketFd, const uint16_t& localPort,
        const std::string& peerAddress, const uint16_t& peerPort
    );

    ssize_t lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) override;
    ssize_t lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) override;

 private:
    void runRx();
    void runTx();

    bool checkTxPipe();

    // Local
    int mSocketFd;
    uint16_t mLocalPort;

    // Peer
    struct sockaddr_in mPeerSockAddr;
    std::mutex mTxMutex;

    std::unique_ptr<std::thread> mpRxThread;
    std::unique_ptr<std::thread> mpTxThread;
    std::atomic<bool> mExitFlag;
};  // class Peer

}   // namespace comm

#include "inline/UdpPeer.inl"

#endif // __UDPPEER_HPP__
