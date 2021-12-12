#include "TcpClient.hpp"

namespace comm {

inline TcpClient::TcpClient(const int& socketFd, const std::string serverAddr, uint16_t remotePort) {
    mExitFlag = false;

    mSocketFd = socketFd;

    mServerAddress = serverAddr;
    mRemotePort = remotePort;

    mpRxThread.reset(new std::thread(&TcpClient::runRx, this));
    mpTxThread.reset(new std::thread(&TcpClient::runTx, this));
}

inline TcpClient::~TcpClient() {
    this->close();
    LOGI("[%s][%d] Finalized!\n", __func__, __LINE__);
}

}   // namespace comm
