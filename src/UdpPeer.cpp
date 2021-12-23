#include "UdpPeer.hpp"

namespace comm {

std::unique_ptr<UdpPeer> UdpPeer::create(
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
        ::close(socketFd);
        return udpPeer;
    }

    int flags = fcntl(socketFd, F_GETFL, 0);
    if (0 > flags) {
        perror("Failed to get socket flags!\n");
        ::close(socketFd);
        return udpPeer;
    }

    if (0 > fcntl(socketFd, F_SETFL, (flags | O_NONBLOCK))) {
        perror("Failed to enable NON-BLOCKING mode!\n");
        ::close(socketFd);
        return udpPeer;
    }

    struct sockaddr_in localSocketAddr;
    localSocketAddr.sin_family      = AF_INET;
    localSocketAddr.sin_addr.s_addr = INADDR_ANY;
    localSocketAddr.sin_port        = htons(localPort);
    int ret = bind(socketFd, (struct sockaddr *)&localSocketAddr, sizeof(localSocketAddr));
    if (0 > ret) {
        perror("Failed to bind socket!\n");
        ::close(socketFd);
        return nullptr;
    }

    LOGI("[%s][%d] Bound at port %u\n", __func__, __LINE__, localPort);
    return std::unique_ptr<UdpPeer>(new UdpPeer(socketFd, localPort, peerAddress, peerPort));
}

void UdpPeer::close() {
    mExitFlag = true;

    if ((mpRxThread) && (mpRxThread->joinable())) {
        mpRxThread->join();
    }

    if ((mpTxThread) && (mpTxThread->joinable())) {
        mpTxThread->join();
    }

    if (0 <= mSocketFd) {
        ::close(mSocketFd);
        mSocketFd = -1;
    }
}

bool UdpPeer::setDestination(const std::string& address, const uint16_t& port) {
    if (address.empty() || (0 == port)) {
        LOGE("[%s][%d] Invalid peer information (`%s`/%u)!\n", __func__, __LINE__, address.c_str(), port);
        return false;
    }

    std::lock_guard<std::mutex> lock(mTxMutex);
    mPeerSockAddr.sin_family      = AF_INET;
    mPeerSockAddr.sin_port        = htons(port);
    mPeerSockAddr.sin_addr.s_addr = inet_addr(address.c_str());

    return true;
}

void UdpPeer::runRx() {
    while(!mExitFlag) {
        if (!proceedRx()) {
            LOGE("[%s][%d] Rx Pipe was broken!\n", __func__, __LINE__);
            break;
        }
    }
}

void UdpPeer::runTx() {
    while(!mExitFlag) {
        if (!proceedTx(!checkTxPipe())) {
            LOGE("[%s][%d] Tx Pipe was broken!\n", __func__, __LINE__);
            break;
        }
    }
}

ssize_t UdpPeer::lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) {
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
    } else if (0 < ret) {
        LOGD("[%s][%d] Received %ld bytes\n", __func__, __LINE__, ret);
    }

    return ret;
}

ssize_t UdpPeer::lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) {
    // Send data over UDP
    ssize_t ret = 0LL;
    for (int i = 0; i < TX_RETRY_COUNT; i++) {
        {
            std::lock_guard<std::mutex> lock(mTxMutex);
            ret = sendto(
                            mSocketFd, pData.get(), size, 0,
                            reinterpret_cast<struct sockaddr *>(&mPeerSockAddr), sizeof(mPeerSockAddr)
                        );
        }

        if (0 > ret) {
            if (EWOULDBLOCK == errno) {
                // Ignore & retry
            } else {
                perror("");
                break;
            }
        } else {
            LOGD("[%s][%d] Transmitted %ld bytes\n", __func__, __LINE__, ret);
            break;
        }
    }

    return ret;
}

}   // namespace comm
