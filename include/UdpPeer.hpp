#ifndef __UDPPEER_HPP__
#define __UDPPEER_HPP__

#ifdef __WIN32__
#include <winsock2.h>
#include <ws2tcpip.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#else // __WIN32__
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#endif   // __WIN32__

#include <cstdint>
#include <sys/types.h>

#include <memory>
#include <mutex>
#include <thread>

#include "common.hpp"
#include "P2P_Endpoint.hpp"
#include "Packet.hpp"

#ifndef __WIN32__
typedef int SOCKET;
#endif  // __WIN32__

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
        const SOCKET& socketFd, const uint16_t& localPort,
        const std::string& peerAddress, const uint16_t& peerPort
    );

    ssize_t lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) override;
    ssize_t lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) override;

 private:
    void runRx();
    void runTx();

    bool checkTxPipe();

    // Local
    SOCKET mSocketFd;
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
