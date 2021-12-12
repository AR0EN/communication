#ifndef __ENPOINT_HPP__
#define __ENPOINT_HPP__

#include <errno.h>
#include <cstdint>
#include <unistd.h>

#include <memory>
#include <mutex>
#include <vector>

#include "Encoder.hpp"
#include "Packet.hpp"
#include "SyncQueue.hpp"

namespace comm {

static constexpr time_t RX_TIMEOUT_S = 1LL;
static constexpr int TX_RETRY_COUNT = 3;

class EndPoint {
 public:
    virtual ~EndPoint() {}

    bool send(std::unique_ptr<Packet>& pPacket);
    bool send(std::unique_ptr<Packet>&& pPacket);
    bool recvAll(std::vector<std::unique_ptr<Packet>>& pRxPackets);

    bool proceedRx();
    bool proceedTx();

 protected:
    EndPoint() { mpRxBuffer.reset(new uint8_t[MAX_FRAME_SIZE]); }

    virtual bool checkRxPipe();
    virtual bool checkTxPipe();

    virtual ssize_t lread(const std::unique_ptr<uint8_t[]>&, const size_t&) = 0;
    virtual ssize_t lwrite(const std::unique_ptr<uint8_t[]>&, const size_t&) = 0;

    std::mutex mRxMutex;
    std::mutex mTxMutex;

 private:
    std::unique_ptr<uint8_t[]> mpRxBuffer;

    Decoder mDecoder;

    dstruct::SyncQueue<Packet> mTxQueue;
};  // class EndPoint

}   // namespace comm

#include "inline/EndPoint.inl"

#endif // __ENPOINT_HPP__
