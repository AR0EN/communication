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
    std::lock_guard<std::mutex> lock(mRemoteSocketMutex);
    return (0 <= mRemoteSocketFd);
}

inline bool TcpServer::checkTxPipe() {
    std::lock_guard<std::mutex> lock(mRemoteSocketMutex);
    return (0 <= mRemoteSocketFd);
}

}   // namespace comm
