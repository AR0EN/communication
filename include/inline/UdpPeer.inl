#include "UdpPeer.hpp"

namespace comm {

inline UdpPeer::UdpPeer(
        const int& socketFd, const uint16_t& localPort,
        const std::string& peerAddress, const uint16_t& peerPort
) {
    mSocketFd = socketFd;
    mLocalPort = localPort;
    mPeerAddress = peerAddress;
    mPeerPort = peerPort;
    mExitFlag = false;
    mpRxThread.reset(new std::thread(&UdpPeer::runRx, this));
    mpTxThread.reset(new std::thread(&UdpPeer::runTx, this));
}

inline UdpPeer::~UdpPeer() {
    stop();
    LOGI("[%s][%d] Finalized!\n", __func__, __LINE__);
}

inline bool UdpPeer::checkRxPipe() {
    return (0 < mSocketFd);
}

inline bool UdpPeer::checkTxPipe() {
    return (!mPeerAddress.empty()) && (0 < mPeerPort);
}

}   // namespace comm
