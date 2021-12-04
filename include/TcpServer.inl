#include "TcpServer.hpp"

namespace comm {

inline TcpServer::TcpServer(int localSocketFd) {
    mLocalSocketFd = localSocketFd;
    mRemoteSocketFd = -1;

    mExitFlag = false;
    mpRxThread.reset(new std::thread(&TcpServer::runRx, this));
    mpTxThread.reset(new std::thread(&TcpServer::runTx, this));
}

inline TcpServer::~TcpServer() {
    stop();
    LOGI("[%s][%d] Finalized!\n", __func__, __LINE__);
}

inline std::unique_ptr<TcpServer> TcpServer::create(uint16_t localPort) {
    std::unique_ptr<TcpServer> tcpServer;

    if (0 == localPort) {
        LOGE("[%s][%d] Local Port must be a positive value!\n", __func__, __LINE__);
        return tcpServer;
    }

    int localSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > localSocketFd) {
        perror("Could not create TCP socket!\n");
        return tcpServer;
    }

    int enable = 1;
    int ret = setsockopt(localSocketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    if (0 > ret) {
        perror("Failed to enable SO_REUSEADDR!\n");
        close(localSocketFd);
        return tcpServer;
    }

    struct timeval timeout;
    timeout.tv_sec = RX_TIMEOUT_S;
    timeout.tv_usec = 0;
    ret = setsockopt (localSocketFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (0 > ret) {
        perror("Failed to configure SO_RCVTIMEO!\n");
        close(localSocketFd);
        return tcpServer;
    }

    struct sockaddr_in localSocketAddr;
    localSocketAddr.sin_family      = AF_INET;
    localSocketAddr.sin_addr.s_addr = INADDR_ANY;
    localSocketAddr.sin_port = htons(localPort);
    ret = bind(localSocketFd, (struct sockaddr *)&localSocketAddr, sizeof(localSocketAddr));
    if (0 > ret) {
        perror("Failed to assigns address to the socket!\n");
        close(localSocketFd);
        return tcpServer;
    }

    ret = listen(localSocketFd, BACKLOG);
    if (0 != ret) {
        perror("Failed to mark the socket as a passive socket!\n");
        close(localSocketFd);
        return tcpServer;
    }

    LOGI("[%s][%d] Tcp Server is listenning at port %u ...\n", __func__, __LINE__, localPort);

    return std::unique_ptr<TcpServer>(new TcpServer(localSocketFd));
}

inline void TcpServer::stop() {
    mExitFlag = true;

    if ((mpRxThread) && (mpRxThread->joinable())) {
        mpRxThread->join();
        mpRxThread.reset();
    }

    if ((mpTxThread) && (mpTxThread->joinable())) {
        mpTxThread->join();
        mpTxThread.reset();
    }

    if (0 <= mLocalSocketFd) {
        close(mLocalSocketFd);
        mLocalSocketFd = -1;
    }
}

inline bool TcpServer::checkRxPipe() {
    std::lock_guard<std::mutex> lock(mRemoteSocketMutex);
    return (0 <= mRemoteSocketFd);
}

inline bool TcpServer::checkTxPipe() {
    std::lock_guard<std::mutex> lock(mRemoteSocketMutex);
    return (0 <= mRemoteSocketFd);
}

inline ssize_t TcpServer::lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) {
    ssize_t ret = read(mRemoteSocketFd, pBuffer.get(), limit);

    if (0 > ret) {
        if (EWOULDBLOCK == errno) {
            ret = 0;
        } else {
            std::lock_guard<std::mutex> lock(mRemoteSocketMutex);
            mRemoteSocketFd = -1;
            perror("");
        }
    } else {
        LOGD("[%s][%d] Received %ld bytes\n", __func__, __LINE__, ret);
    }

    return ret;
}

inline ssize_t TcpServer::lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) {
    // Send data over TCP
    ssize_t ret = 0LL;
    for (int i = 0; i < TX_RETRY_COUNT; i++) {
        ret = write(mRemoteSocketFd, pData.get(), size);

        if (0 > ret) {
            if (EWOULDBLOCK != errno) {
                std::lock_guard<std::mutex> lock(mRemoteSocketMutex);
                mRemoteSocketFd = -1;
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

inline void TcpServer::runRx() {
    struct sockaddr_in remoteSocketAddr;
    socklen_t remoteAddressSize = static_cast<socklen_t>(sizeof(remoteSocketAddr));

    while(!mExitFlag) {
        {
            std::lock_guard<std::mutex> lock(mRemoteSocketMutex);
            mRemoteSocketFd = accept(
                                    mLocalSocketFd,
                                    reinterpret_cast<struct sockaddr*>(&remoteSocketAddr),
                                    &remoteAddressSize
                                );

            if (0 > mRemoteSocketFd) {
                if (EAGAIN == errno) {
                    // Timeout - do nothing
                    continue;
                } else {
                    perror("Could not access connection queue!\n");
                    break;
                }
            }
        }

        while(!mExitFlag) {
            if (!proceedRx()) {
                LOGE("[%s][%d]\n", __func__, __LINE__);
                break;
            }
        }

        {
            std::lock_guard<std::mutex> lock(mRemoteSocketMutex);
            close(mRemoteSocketFd);
            mRemoteSocketFd = -1;
        }
    }
}

inline void TcpServer::runTx() {
    while(!mExitFlag) {
        if (!proceedTx()) {
            LOGE("[%s][%d]\n", __func__, __LINE__);
            break;
        }
    }
}

}   // namespace comm
