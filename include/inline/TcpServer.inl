#include "TcpServer.hpp"

namespace comm {

inline TcpServer::TcpServer(int localSocketFd) {
    mExitFlag = false;

    mLocalSocketFd = localSocketFd;
    mRxPipeFd = -1;
    mTxPipeFd = -1;

    mpRxThread.reset(new std::thread(&TcpServer::runRx, this));
    mpTxThread.reset(new std::thread(&TcpServer::runTx, this));
}

inline TcpServer::~TcpServer() {
    this->close();
    LOGI("[%s][%d] Finalized!\n", __func__, __LINE__);
}

inline bool TcpServer::isClientConnected() {
    return checkTxPipe();
}

inline bool TcpServer::checkRxPipe() {
    return (0 < mRxPipeFd);
}

inline bool TcpServer::checkTxPipe() {
    return (0 < mTxPipeFd);
}

}   // namespace comm
