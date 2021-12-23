#include "TcpServer.hpp"

namespace comm {

std::unique_ptr<TcpServer> TcpServer::create(uint16_t localPort) {
    std::unique_ptr<TcpServer> tcpServer;

    if (0 == localPort) {
        LOGE("[%s][%d] Local Port must be a positive value!\n", __func__, __LINE__);
        return tcpServer;
    }

    int ret;

#ifdef __WIN32__
    //---------------------------------------
    // Initialize Winsock
    WSADATA wsaData;
    ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (NO_ERROR != ret) {
        LOGE("WSAStartup() failed (error code: %d)!\n", ret);
        return tcpServer;
    }
#endif  // __WIN32__

    SOCKET localSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == localSocketFd) {
#ifdef __WIN32__
        LOGE("socket() failed (error code: %d)!\n", WSAGetLastError());
        WSACleanup();
#else   // __WIN32__
        perror("Could not create TCP socket!\n");
#endif  // __WIN32__
        return tcpServer;
    }

#ifdef __WIN32__
    BOOL enable = TRUE;
    ret = setsockopt(localSocketFd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&enable), sizeof(enable));
    if (SOCKET_ERROR == ret) {
        LOGE("Failed to enable SO_REUSEADDR (error code: %d)\n", WSAGetLastError());
        closesocket(localSocketFd);
        WSACleanup();
        return tcpServer;
    }
#else   // __WIN32__
    int enable = 1;
    ret = setsockopt(localSocketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    if (0 > ret) {
        perror("Failed to enable SO_REUSEADDR!\n");
        ::close(localSocketFd);
        return tcpServer;
    }
#endif  // __WIN32__

#ifdef __WIN32__
    DWORD timeout = RX_TIMEOUT_S * 1000;
    ret = setsockopt (localSocketFd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&timeout), sizeof(timeout));
    if (SOCKET_ERROR == ret) {
        LOGE("Failed to configure SO_RCVTIMEO (error code: %d)\n", WSAGetLastError());
        closesocket(localSocketFd);
        WSACleanup();
        return tcpServer;
    }
#else   // __WIN32__
    struct timeval timeout;
    timeout.tv_sec = RX_TIMEOUT_S;
    timeout.tv_usec = 0;
    ret = setsockopt (localSocketFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (0 > ret) {
        perror("Failed to configure SO_RCVTIMEO!\n");
        ::close(localSocketFd);
        return tcpServer;
    }
#endif  // __WIN32__

    struct sockaddr_in localSocketAddr;
    localSocketAddr.sin_family      = AF_INET;
    localSocketAddr.sin_addr.s_addr = INADDR_ANY;
    localSocketAddr.sin_port = htons(localPort);
#ifdef __WIN32__
    ret = bind(localSocketFd, reinterpret_cast<const struct sockaddr *>(&localSocketAddr), sizeof(localSocketAddr));
    if (SOCKET_ERROR == ret) {
        LOGE("Failed to assigns address to the socket (error code: %d)\n", WSAGetLastError());
        closesocket(localSocketFd);
        WSACleanup();
        return tcpServer;
    }
#else   // __WIN32__
    ret = bind(localSocketFd, reinterpret_cast<const struct sockaddr *>(&localSocketAddr), sizeof(localSocketAddr));
    if (0 > ret) {
        perror("Failed to assigns address to the socket!\n");
        ::close(localSocketFd);
        return tcpServer;
    }
#endif  // __WIN32__

    ret = listen(localSocketFd, BACKLOG);
#ifdef __WIN32__
    if (SOCKET_ERROR == ret) {
        LOGE("Failed to mark the socket as a passive socket (error code: %d)\n", WSAGetLastError());
        closesocket(localSocketFd);
        WSACleanup();
        return tcpServer;
    }
#else   // __WIN32__
    if (0 != ret) {
        perror("Failed to mark the socket as a passive socket!\n");
        ::close(localSocketFd);
        return tcpServer;
    }
#endif  // __WIN32__

    LOGI("[%s][%d] Tcp Server is listenning at port %u ...\n", __func__, __LINE__, localPort);

    return std::unique_ptr<TcpServer>(new TcpServer(localSocketFd));
}

void TcpServer::close() {
    mExitFlag = true;

    if ((mpRxThread) && (mpRxThread->joinable())) {
        mpRxThread->join();
    }

    if ((mpTxThread) && (mpTxThread->joinable())) {
        mpTxThread->join();
    }

    if (0 <= mLocalSocketFd) {
#ifdef __WIN32__
        closesocket(mLocalSocketFd);
        WSACleanup();
#else   // __WIN32__
        ::close(mLocalSocketFd);
#endif  // __WIN32__
        mLocalSocketFd = -1;
    }
}

void TcpServer::runRx() {
    struct sockaddr_in remoteSocketAddr;
    socklen_t remoteAddressSize = static_cast<socklen_t>(sizeof(remoteSocketAddr));

    while(!mExitFlag) {
        mRxPipeFd = accept(
                                mLocalSocketFd,
                                reinterpret_cast<struct sockaddr*>(&remoteSocketAddr),
                                &remoteAddressSize
                            );

        if (!checkRxPipe()) {
            if (EAGAIN == errno) {
                // Timeout - do nothing
                continue;
            } else {
                perror("Could not access connection queue!\n");
                break;
            }
        }

        mTxPipeFd = mRxPipeFd;

        while(!mExitFlag) {
            if (!proceedRx()) {
                LOGE("[%s][%d] Rx Pipe was broken!\n", __func__, __LINE__);
                break;
            }
        }

        mTxPipeFd = -1;
#ifdef __WIN32__
        closesocket(mRxPipeFd);
#else   // __WIN32__
        ::close(mRxPipeFd);
#endif  // __WIN32__
    }
}

void TcpServer::runTx() {
    while(!mExitFlag) {
        if (!proceedTx(!checkTxPipe())) {
            LOGE("[%s][%d] Tx Pipe was broken!\n", __func__, __LINE__);
        }
    }
}

ssize_t TcpServer::lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) {
    ssize_t ret = read(mRxPipeFd, pBuffer.get(), limit);

#ifdef __WIN32__
    if (SOCKET_ERROR == ret) {
        int error = WSAGetLastError();
        if (WSAEWOULDBLOCK == error) {
            ret = 0;
        } else {
            LOGE("Failed to read from Tcp Socket (error code: %d)\n", error);
        }
    } else if (0 < ret) {
        LOGD("[%s][%d] Received %ld bytes\n", __func__, __LINE__, ret);
    }
#else   // __WIN32__
    if (0 > ret) {
        if (EWOULDBLOCK == errno) {
            ret = 0;
        } else {
            perror("Failed to read from Tcp Socket!");
        }
    } else if (0 < ret) {
        LOGD("[%s][%d] Received %ld bytes\n", __func__, __LINE__, ret);
    }
#endif  // __WIN32__

    return ret;
}

ssize_t TcpServer::lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) {
    // Send data over TCP
    ssize_t ret = 0LL;
    for (int i = 0; i < TX_RETRY_COUNT; i++) {
        ret = write(mTxPipeFd, pData.get(), size);

#ifdef __WIN32__
        if (SOCKET_ERROR == ret) {
            int error = WSAGetLastError();
            if (WSAEWOULDBLOCK == error) {
                // Ignore & retry
            } else {
                mTxPipeFd = -1;
                LOGE("Failed to write to Tcp Socket (error code: %d)\n", error);
                break;                
            }
        } else if (0 < ret) {
            LOGD("[%s][%d] Transmitted %ld bytes\n", __func__, __LINE__, ret);
            break;
        }
#else   // __WIN32__
        if (0 > ret) {
            if (EWOULDBLOCK == errno) {
                // Ignore & retry
            } else {
                mTxPipeFd = -1;
                perror("Failed to write to Tcp Socket!");
                break;
            }
        } else if (0 < ret) {
            LOGD("[%s][%d] Transmitted %ld bytes\n", __func__, __LINE__, ret);
            break;
        }
#endif  // __WIN32__
    }

    return ret;
}

}   // namespace comm
