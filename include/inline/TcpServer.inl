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

inline bool TcpServer::checkRxPipe() {
    return (0 <= mRemoteSocketFd);
}

inline bool TcpServer::checkTxPipe() {
    return (0 <= mRemoteSocketFd);
}

}   // namespace comm
