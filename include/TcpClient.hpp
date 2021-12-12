#ifndef __TCPCLIENT_HPP__
#define __TCPCLIENT_HPP__

#include <arpa/inet.h>
#include <cstdint>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <atomic>
#include <memory>
#include <string>
#include <thread>

#include "common.hpp"
#include "Packet.hpp"
#include "P2P_EndPoint.hpp"

namespace comm {

class TcpClient : public P2P_EndPoint {
 public:
    void close() override;

    virtual ~TcpClient();
    static std::unique_ptr<TcpClient> create(const std::string& serverAddr, const uint16_t& remotePort);

 protected:
    TcpClient(const int& socketFd, const std::string serverAddr, uint16_t remotePort);

    ssize_t lread(const std::unique_ptr<uint8_t[]>& pBuffer, const size_t& limit) override;
    ssize_t lwrite(const std::unique_ptr<uint8_t[]>& pData, const size_t& size) override;

 private:
    void runRx();
    void runTx();

    std::string mServerAddress;
    uint16_t mRemotePort;
    int mSocketFd;

    std::unique_ptr<std::thread> mpRxThread;
    std::unique_ptr<std::thread> mpTxThread;
    std::atomic<bool> mExitFlag;
};  // class TcpClient

}   // namespace comm

#include "inline/TcpClient.inl"

#endif // __TCPCLIENT_HPP__
