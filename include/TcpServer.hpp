#ifndef __TCPSERVER_HPP__
#define __TCPSERVER_HPP__

#include <arpa/inet.h>
#include <cstdint>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

#include "common.hpp"
#include "P2P_Endpoint.hpp"
#include "Packet.hpp"

namespace comm {

class TcpServer : public P2P_Endpoint {
 public:
    bool isPeerConnected() override;
    void close() override;

    ~TcpServer();
    static std::unique_ptr<TcpServer> create(uint16_t localPort);

 protected:
    TcpServer(int localSocketFd);

    ssize_t lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) override;
    ssize_t lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) override;

 private:
    void runRx();
    void runTx();

    bool checkRxPipe();
    bool checkTxPipe();

    int mLocalSocketFd;
    // TcpServer accept only one client at a time,
    // therefore, accept & read take place in the same thread
    // -> no need to use std::atomic for Rx Pipe
    int mRxPipeFd;
    std::atomic<int> mTxPipeFd;

    std::unique_ptr<std::thread> mpRxThread;
    std::unique_ptr<std::thread> mpTxThread;
    std::atomic<bool> mExitFlag;

    static constexpr int BACKLOG = 1;
};  // class TcpServer

}   // namespace comm

#include "inline/TcpServer.inl"

#endif // __TCPSERVER_HPP__
