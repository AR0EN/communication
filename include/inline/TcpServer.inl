#include "TcpServer.hpp"

namespace comm {

inline TcpServer::TcpServer(SOCKET localSocketFd) {
    mExitFlag = false;

    mLocalSocketFd = localSocketFd;
    mRxPipeFd = INVALID_SOCKET;
    mTxPipeFd = static_cast<unsigned>(INVALID_SOCKET);

    mpRxThread.reset(new std::thread(&TcpServer::runRx, this));
    mpTxThread.reset(new std::thread(&TcpServer::runTx, this));
}

inline TcpServer::~TcpServer() {
    this->close();
    LOGI("[%s][%d] Finalized!\n", __func__, __LINE__);
}

inline bool TcpServer::isPeerConnected() {
    return checkTxPipe();
}

inline bool TcpServer::checkRxPipe() {
#ifdef __WIN32__
    return (0 < mRxPipeFd) && (INVALID_SOCKET != mRxPipeFd);
#else   // __WIN32__
    return (0 < mRxPipeFd);
#endif  // __WIN32__
}

inline bool TcpServer::checkTxPipe() {
#ifdef __WIN32__
    return (0 < mRxPipeFd) && (INVALID_SOCKET != mRxPipeFd);
#else   // __WIN32__
    return (0 < mTxPipeFd);
#endif  // __WIN32__
}

}   // namespace comm
