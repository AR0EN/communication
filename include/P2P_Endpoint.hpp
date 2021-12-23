#ifndef __P2P_ENPOINT_HPP__
#define __P2P_ENPOINT_HPP__

#include <errno.h>
#include <cstdint>
#include <unistd.h>

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>

#ifdef __WIN32__
#include <windows.h>
#endif   // __WIN32__

#include "Encoder.hpp"
#include "Packet.hpp"
#include "SyncQueue.hpp"

namespace comm {

#ifdef __WIN32__
static constexpr DWORD RX_TIMEOUT_S = 1;
#else // __WIN32__
static constexpr time_t RX_TIMEOUT_S = 1LL;
#endif   // __WIN32__
static constexpr int TX_RETRY_COUNT = 3;

class P2P_Endpoint {
 public:
    virtual ~P2P_Endpoint() {}

    bool send(std::unique_ptr<Packet>& pPacket);
    bool send(std::unique_ptr<Packet>&& pPacket);
    bool recvAll(std::deque<std::unique_ptr<Packet>>& pRxPackets, bool wait=true);
    virtual bool isPeerConnected();

    virtual void close() = 0;

 protected:
    P2P_Endpoint() {
        mpRxBuffer.reset(new uint8_t[MAX_FRAME_SIZE]);
        mTransactionId = 0;
    }

    bool proceedRx();
    bool proceedTx();

    virtual ssize_t lread(const std::unique_ptr<uint8_t[]>&, const size_t&) = 0;
    virtual ssize_t lwrite(const std::unique_ptr<uint8_t[]>&, const size_t&) = 0;

 private:
    std::unique_ptr<uint8_t[]> mpRxBuffer;
    Decoder mDecoder;
    std::mutex mRxMutex;

    dstruct::SyncQueue<Packet> mTxQueue;
    uint16_t mTransactionId;
    std::mutex mTxMutex;
};  // class P2P_Endpoint

}   // namespace comm

#include "inline/P2P_Endpoint.inl"

#endif // __P2P_ENPOINT_HPP__
