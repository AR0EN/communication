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
#include "EndPoint.hpp"
#include "Packet.hpp"

namespace comm {

class TcpServer : public EndPoint {
 public:
    bool checkRxPipe() override ;
    bool checkTxPipe() override ;

    void stop();

    ~TcpServer();
    static std::unique_ptr<TcpServer> create(uint16_t localPort);

 protected:
    TcpServer(int localSocketFd);

    ssize_t lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) override;
    ssize_t lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) override;

 private:
    void runRx();
    void runTx();

    int mLocalSocketFd;
    int mRemoteSocketFd;
    std::mutex mRemoteSocketMutex;

    std::unique_ptr<std::thread> mpRxThread;
    std::unique_ptr<std::thread> mpTxThread;
    std::atomic<bool> mExitFlag;

    static constexpr int BACKLOG = 1;
};  // class TcpServer

}   // namespace comm

#include "inline/TcpServer.inl"

#endif // __TCPSERVER_HPP__
