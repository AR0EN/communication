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
#include "EndPoint.hpp"
#include "Packet.hpp"

namespace comm {

class UdpPeer : public EndPoint {
 public:
    bool setDestination(const std::string& address, const uint16_t& port);

    bool checkRxPipe() override;
    bool checkTxPipe() override;

    void stop();

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

#include "inline/UdpPeer.inl"

#endif // __UDPPEER_HPP__
