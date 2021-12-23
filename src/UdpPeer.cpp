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

    int ret;

#ifdef __WIN32__
    //---------------------------------------
    // Initialize Winsock
    WSADATA wsaData;
    ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (NO_ERROR != ret) {
        LOGE("WSAStartup() failed (error code: %d)!\n", ret);
        return udpPeer;
    }
#endif  // __WIN32__

    SOCKET socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (INVALID_SOCKET == socketFd) {
#ifdef __WIN32__
        LOGE("socket() failed (error code: %d)!\n", WSAGetLastError());
        WSACleanup();
#else   // __WIN32__
        perror("Could not create UDP socket!\n");
#endif  // __WIN32__
        return udpPeer;
    }

#ifdef __WIN32__
    BOOL enable = TRUE;
    ret = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&enable), sizeof(enable));
    if (SOCKET_ERROR == ret) {
        LOGE("Failed to enable SO_REUSEADDR (error code: %d)\n", WSAGetLastError());
        closesocket(socketFd);
        WSACleanup();
        return udpPeer;
    }
#else   // __WIN32__
    int enable = 1;
    ret = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    if (0 > ret) {
        perror("Failed to enable SO_REUSEADDR!\n");
        ::close(socketFd);
        return udpPeer;
    }
#endif  // __WIN32__

#ifdef __WIN32__
    DWORD timeout = RX_TIMEOUT_S * 1000;
    ret = setsockopt (socketFd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&timeout), sizeof(timeout));
    if (SOCKET_ERROR == ret) {
        LOGE("Failed to configure SO_RCVTIMEO (error code: %d)\n", WSAGetLastError());
        closesocket(socketFd);
        WSACleanup();
        return udpPeer;
    }
#else   // __WIN32__
    struct timeval timeout;
    timeout.tv_sec = RX_TIMEOUT_S;
    timeout.tv_usec = 0;
    ret = setsockopt (socketFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (0 > ret) {
        perror("Failed to configure SO_RCVTIMEO!\n");
        ::close(socketFd);
        return udpPeer;
    }
#endif  // __WIN32__

    struct sockaddr_in localSocketAddr;
    localSocketAddr.sin_family      = AF_INET;
    localSocketAddr.sin_addr.s_addr = INADDR_ANY;
    localSocketAddr.sin_port        = htons(localPort);
#ifdef __WIN32__
    ret = bind(socketFd, reinterpret_cast<const struct sockaddr *>(&localSocketAddr), sizeof(localSocketAddr));
    if (SOCKET_ERROR == ret) {
        LOGE("Failed to assigns address to the socket (error code: %d)\n", WSAGetLastError());
        closesocket(socketFd);
        WSACleanup();
        return udpPeer;
    }
#else   // __WIN32__
    ret = bind(socketFd, reinterpret_cast<const struct sockaddr *>(&localSocketAddr), sizeof(localSocketAddr));
    if (0 > ret) {
        perror("Failed to assigns address to the socket!\n");
        ::close(socketFd);
        return udpPeer;
    }
#endif  // __WIN32__

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
#ifdef __WIN32__
        closesocket(mSocketFd);
        WSACleanup();
#else   // __WIN32__
        ::close(mSocketFd);
#endif  // __WIN32__
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
        if (!checkTxPipe()) {
            continue;
        }

        if (!proceedTx()) {
            LOGE("[%s][%d] Tx Pipe was broken!\n", __func__, __LINE__);
            break;
        }
    }
}

ssize_t UdpPeer::lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) {
    socklen_t remoteAddressSize;
    struct sockaddr_in remoteSocketAddr;
    remoteAddressSize = (socklen_t)sizeof(remoteSocketAddr);

#ifdef __WIN32__
    ssize_t ret = recvfrom(
                    mSocketFd, reinterpret_cast<char *>(pBuffer.get()), limit, 0,
                    reinterpret_cast<struct sockaddr *>(&remoteSocketAddr), &remoteAddressSize
                );

    if (SOCKET_ERROR == ret) {
        int error = WSAGetLastError();
        if (WSAEWOULDBLOCK == error) {
            ret = 0;
        } else {
            LOGE("Failed to read from UDP Socket (error code: %d)\n", error);
        }
    } else if (0 < ret) {
        LOGD("[%s][%d] Received %ld bytes\n", __func__, __LINE__, ret);
    }
#else   // __WIN32__
    ssize_t ret = recvfrom(
                    mSocketFd, pBuffer.get(), limit, 0,
                    reinterpret_cast<struct sockaddr *>(&remoteSocketAddr), &remoteAddressSize
                );

    if (0 > ret) {
        if (EWOULDBLOCK == errno) {
            ret = 0;
        } else {
            perror("Failed to read from UDP Socket!");
        }
    } else if (0 < ret) {
        LOGD("[%s][%d] Received %ld bytes\n", __func__, __LINE__, ret);
    }
#endif  // __WIN32__

    return ret;
}

ssize_t UdpPeer::lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) {
    // Send data over UDP
    ssize_t ret = 0LL;
    for (int i = 0; i < TX_RETRY_COUNT; i++) {
        {
            std::lock_guard<std::mutex> lock(mTxMutex);
#ifdef __WIN32__
            ret = sendto(
                            mSocketFd, reinterpret_cast<const char *>(pData.get()), size, 0,
                            reinterpret_cast<struct sockaddr *>(&mPeerSockAddr), sizeof(mPeerSockAddr)
                        );

            if (SOCKET_ERROR == ret) {
                int error = WSAGetLastError();
                if (WSAEWOULDBLOCK == error) {
                    // Ignore & retry
                } else {
                    LOGE("Failed to write to UDP Socket (error code: %d)\n", error);
                }
            } else if (0 < ret) {
                LOGD("[%s][%d] Transmitted %ld bytes\n", __func__, __LINE__, ret);
                break;
            }
#else   // __WIN32__
            ret = sendto(
                            mSocketFd, pData.get(), size, 0,
                            reinterpret_cast<struct sockaddr *>(&mPeerSockAddr), sizeof(mPeerSockAddr)
                        );

            if (0 > ret) {
                if (EWOULDBLOCK == errno) {
                    // Ignore & retry
                } else {
                    perror("Failed to write to UDP Socket!");
                    break;
                }
            } else if (0 < ret) {
                LOGD("[%s][%d] Transmitted %ld bytes\n", __func__, __LINE__, ret);
                break;
            }
#endif  // __WIN32__
        }        
    }

    return ret;
}

}   // namespace comm
