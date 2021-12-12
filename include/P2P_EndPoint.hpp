#ifndef __ENPOINT_HPP__
#define __ENPOINT_HPP__

#include <errno.h>
#include <cstdint>
#include <unistd.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

#include "Encoder.hpp"
#include "Packet.hpp"
#include "SyncQueue.hpp"

namespace comm {

static constexpr time_t RX_TIMEOUT_S = 1LL;
static constexpr int TX_RETRY_COUNT = 3;

class P2P_EndPoint {
 public:
    virtual ~P2P_EndPoint() {}

    bool send(std::unique_ptr<Packet>& pPacket);
    bool send(std::unique_ptr<Packet>&& pPacket);
    bool recvAll(std::vector<std::unique_ptr<Packet>>& pRxPackets);

    virtual void close() = 0;

 protected:
    P2P_EndPoint() {
        mpRxBuffer.reset(new uint8_t[MAX_FRAME_SIZE]);
    }

    bool proceedRx();
    bool proceedTx();

    virtual ssize_t lread(const std::unique_ptr<uint8_t[]>&, const size_t&) = 0;
    virtual ssize_t lwrite(const std::unique_ptr<uint8_t[]>&, const size_t&) = 0;

 private:
    std::unique_ptr<uint8_t[]> mpRxBuffer;
    Decoder mDecoder;

    dstruct::SyncQueue<Packet> mTxQueue;
};  // class P2P_EndPoint

}   // namespace comm

#include "inline/P2P_EndPoint.inl"

#endif // __ENPOINT_HPP__
