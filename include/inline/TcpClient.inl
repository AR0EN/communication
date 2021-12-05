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

inline bool TcpClient::checkRxPipe() {
    return (0 <= mSocketFd);
}

inline bool TcpClient::checkTxPipe() {
    return (0 <= mSocketFd);
}

}   // namespace comm
