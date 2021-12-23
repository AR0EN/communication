#include "TcpClient.hpp"

namespace comm {

std::unique_ptr<TcpClient> TcpClient::create(const std::string& serverAddr, const uint16_t& remotePort) {
    std::unique_ptr<TcpClient> tcpClient;

    if (serverAddr.empty()) {
        LOGE("[%s][%d] Server 's Address is invalid!\n", __func__, __LINE__);
        return tcpClient;
    }

    if (0 == remotePort) {
        LOGE("[%s][%d] Server 's Port must be a positive value!\n", __func__, __LINE__);
        return tcpClient;
    }

    int ret;

#ifdef __WIN32__
    //---------------------------------------
    // Initialize Winsock
    WSADATA wsaData;
    ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (NO_ERROR != ret) {
        LOGE("WSAStartup() failed (error code: %d)!\n", ret);
        return tcpClient;
    }
#endif  // __WIN32__

    SOCKET socketFd = socket(AF_INET, SOCK_STREAM, 0);
#ifdef __WIN32__
    if (INVALID_SOCKET == socketFd) {
        LOGE("socket() failed (error code: %d)!\n", WSAGetLastError());
        WSACleanup();
        return tcpClient;
    }
#else   // __WIN32__
    if (0 > socketFd) {
        perror("Could not create TCP socket!\n");
        return tcpClient;
    }
#endif  // __WIN32__

#ifdef __WIN32__
    BOOL enable = TRUE;
    ret = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&enable), sizeof(enable));
    if (SOCKET_ERROR == ret) {
        LOGE("Failed to enable SO_REUSEADDR (error code: %d)\n", WSAGetLastError());
        closesocket(socketFd);
        WSACleanup();
        return tcpClient;
    }
#else   // __WIN32__
    int enable = 1;
    ret = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    if (0 > ret) {
        perror("Failed to enable SO_REUSEADDR!\n");
        ::close(socketFd);
        return tcpClient;
    }
#endif  // __WIN32__

#ifdef __WIN32__
    DWORD timeout = RX_TIMEOUT_S * 1000U;
    ret = setsockopt (socketFd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&timeout), sizeof(timeout));
    if (SOCKET_ERROR == ret) {
        LOGE("Failed to configure SO_RCVTIMEO (error code: %d)\n", WSAGetLastError());
        closesocket(socketFd);
        WSACleanup();
        return tcpClient;
    }
#else   // __WIN32__
    struct timeval timeout;
    timeout.tv_sec = RX_TIMEOUT_S;
    timeout.tv_usec = 0;
    ret = setsockopt (socketFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (0 > ret) {
        perror("Failed to configure SO_RCVTIMEO!\n");
        ::close(socketFd);
        return tcpClient;
    }
#endif  // __WIN32__

    struct sockaddr_in remoteSocketAddr;
    remoteSocketAddr.sin_family      = AF_INET;
    remoteSocketAddr.sin_addr.s_addr = inet_addr(serverAddr.c_str());
    remoteSocketAddr.sin_port        = htons(remotePort);
    ret = connect(socketFd, reinterpret_cast<const struct sockaddr *>(&remoteSocketAddr), sizeof(remoteSocketAddr));
#ifdef __WIN32__
    if (SOCKET_ERROR == ret) {
        LOGE("Failed to connect to %s/%u (error code: %d)\n", serverAddr.c_str(), remotePort, WSAGetLastError());
        closesocket(socketFd);
        WSACleanup();
        return tcpClient;
    }
#else   // __WIN32__
    if (0 != ret) {
        perror("Failed to connect to server!\n");
        ::close(socketFd);
        return tcpClient;
    }
#endif  // __WIN32__

    LOGI("[%s][%d] Connected to %s/%u\n", __func__, __LINE__, serverAddr.c_str(), remotePort);

#ifdef __WIN32__
    unsigned long non_blocking = 1;
    ret = ioctlsocket(socketFd, FIONBIO, &non_blocking);
    if (NO_ERROR != ret) {
        LOGE("Failed to enable NON-BLOCKING mode (error code: %d)\n", WSAGetLastError());
        closesocket(socketFd);
        WSACleanup();
        return tcpClient;
    }
#else   // __WIN32__
    int flags = fcntl(socketFd, F_GETFL, 0);
    if (0 > flags) {
        perror("Failed to get socket flags!\n");
        ::close(socketFd);
        return tcpClient;
    }

    if (0 > fcntl(socketFd, F_SETFL, (flags | O_NONBLOCK))) {
        perror("Failed to enable NON-BLOCKING mode!\n");
        ::close(socketFd);
        return tcpClient;
    }
#endif  // __WIN32__

    return std::unique_ptr<TcpClient>(new TcpClient(socketFd, serverAddr, remotePort));
}

void TcpClient::close() {
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

void TcpClient::runRx() {
    while(!mExitFlag) {
        if (!proceedRx()) {
            LOGE("[%s][%d] Rx Pipe was broken!\n", __func__, __LINE__);
            break;
        }
    }
}

void TcpClient::runTx() {
    while(!mExitFlag) {
        if (!proceedTx()) {
            LOGE("[%s][%d] Tx Pipe was broken!\n", __func__, __LINE__);
            break;
        }
    }
}

ssize_t TcpClient::lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) {
#ifdef __WIN32__
    ssize_t ret = ::recv(mSocketFd, reinterpret_cast<char *>(pBuffer.get()), limit, 0);
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
    ssize_t ret = ::recv(mSocketFd, pBuffer.get(), limit, 0);
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

ssize_t TcpClient::lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) {
    // Send data over TCP
    ssize_t ret = 0LL;
    for (int i = 0; i < TX_RETRY_COUNT; i++) {
#ifdef __WIN32__
        ret = ::send(mSocketFd, reinterpret_cast<const char *>(pData.get()), size, 0);
        if (SOCKET_ERROR == ret) {
            int error = WSAGetLastError();
            if (WSAEWOULDBLOCK == error) {
                // Ignore & retry
            } else {
                LOGE("Failed to write to Tcp Socket (error code: %d)\n", error);
            }
        } else if (0 < ret) {
            LOGD("[%s][%d] Transmitted %ld bytes\n", __func__, __LINE__, ret);
            break;
        }
#else   // __WIN32__
        ret = ::send(mSocketFd, pData.get(), size, 0);
        if (0 > ret) {
            if (EWOULDBLOCK == errno) {
                // Ignore & retry
            } else {
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
