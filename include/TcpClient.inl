#include "TcpClient.hpp"

namespace comm {

inline TcpClient::TcpClient(const int& socketFd, const std::string serverAddr, uint16_t remotePort) {
    mSocketFd = socketFd;
    mServerAddress = serverAddr;
    mRemotePort = remotePort;

    mExitFlag = false;
    mpRxThread.reset(new std::thread(&TcpClient::runRx, this));
    mpTxThread.reset(new std::thread(&TcpClient::runTx, this));
}

inline TcpClient::~TcpClient() {
    stop();
    LOGI("[%s][%d] Finalized!\n", __func__, __LINE__);
}

inline std::unique_ptr<TcpClient> TcpClient::create(const std::string& serverAddr, const uint16_t& remotePort) {
    std::unique_ptr<TcpClient> tcpClient;

    if (serverAddr.empty()) {
        LOGE("[%s][%d] Server 's Address is invalid!\n", __func__, __LINE__);
        return tcpClient;
    }

    if (0 == remotePort) {
        LOGE("[%s][%d] Server 's Port must be a positive value!\n", __func__, __LINE__);
        return tcpClient;
    }

    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > socketFd) {
        perror("Could not create TCP socket!\n");
        return tcpClient;
    }

    int enable = 1;
    int ret = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    if (0 > ret) {
        perror("Failed to enable SO_REUSEADDR!\n");
        close(socketFd);
        return tcpClient;
    }

    struct timeval timeout;
    timeout.tv_sec = RX_TIMEOUT_S;
    timeout.tv_usec = 0;
    ret = setsockopt (socketFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (0 > ret) {
        perror("Failed to configure SO_RCVTIMEO!\n");
        close(socketFd);
        return tcpClient;
    }

    struct sockaddr_in remoteSocketAddr;
    remoteSocketAddr.sin_family      = AF_INET;
    remoteSocketAddr.sin_addr.s_addr = inet_addr(serverAddr.c_str());
    remoteSocketAddr.sin_port        = htons(remotePort);
    ret = connect(socketFd, (struct sockaddr *)&remoteSocketAddr, sizeof(remoteSocketAddr));
    if (0 != ret) {
        perror("Failed to connect to server!\n");
        close(socketFd);
        return tcpClient;
    }

    LOGI("[%s][%d] Connected to %s/%u\n", __func__, __LINE__, serverAddr.c_str(), remotePort);

    return std::unique_ptr<TcpClient>(new TcpClient(socketFd, serverAddr, remotePort));
}

inline void TcpClient::stop() {
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

inline bool TcpClient::checkRxPipe() {
    return (0 <= mSocketFd);
}

inline bool TcpClient::checkTxPipe() {
    return (0 <= mSocketFd);
}

inline ssize_t TcpClient::lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) {
    ssize_t ret = read(mSocketFd, pBuffer.get(), limit);

    if (0 > ret) {
        if (EWOULDBLOCK == errno) {
            ret = 0;
        } else {
            mSocketFd = -1;
            perror("");
        }
    } else {
        LOGD("[%s][%d] Received %ld bytes\n", __func__, __LINE__, ret);
    }

    return ret;
}

inline ssize_t TcpClient::lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) {
    // Send data over TCP
    ssize_t ret = 0LL;
    for (int i = 0; i < TX_RETRY_COUNT; i++) {
        ret = write(mSocketFd, pData.get(), size);

        if (0 > ret) {
            if (EWOULDBLOCK == errno) {
                // Ignore & retry
            } else {
                mSocketFd = -1;
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

inline void TcpClient::runRx() {
    while(!mExitFlag) {
        if (!proceedRx()) {
            LOGE("[%s][%d]\n", __func__, __LINE__);
            break;
        }
    }
}

inline void TcpClient::runTx() {
    while(!mExitFlag) {
        if (!proceedTx()) {
            LOGE("[%s][%d]\n", __func__, __LINE__);
            break;
        }
    }
}

}   // namespace comm
