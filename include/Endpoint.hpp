#ifndef __ENPOINT_HPP__
#define __ENPOINT_HPP__

#include <errno.h>
#include <cstdint>
#include <cstudio>
#include <unistd.h>

#include <memory>
#include <mutex>
#include <vector>

#include "Packet.hpp"
#include "SyncQueue.hpp"

namespace comm {

class EndPoint {
 public:
    virtual ~EndPoint() {}

    bool send(std::unique_ptr<Packet>& pPacket) {
        if (pPacket) {
            mTxQueue.enqueue(pPacket);
            return true;
        } else {
            fprintf(stderr, "[%s][%d] Tx packet must not be empty!\n", __func__, __LINE__);
            return false;
        }
    }

    bool consumeRx(std::vector<std::unique_ptr<Packet>>& pRxPackets) {
        return mDecoder.dequeue(pRxPackets);
    }

    virtual bool checkRxPipe() {
        return true;
    }

    virtual bool checkTxPipe() {
        return true;
    }

 protected:
    EndPoint() {
        mpRxBuffer.reset(new uint8_t[MAX_FRAME_SIZE]);
    }

    inline bool proceedRx() {
        ssize_t byteCount = 0;

        {
            std::lock_guard<std::mutex> lock(mRxMutex);

            if (!checkRxPipe()) {
#ifdef DEBUG
                fprintf(stderr, "[%s][%d] Rx pipe is closed!\n", __func__, __LINE__);
#endif  // DEBUG
                return true;
            }

            byteCount = lread(mpRxBuffer, MAX_FRAME_SIZE);
        }

        if (0 > byteCount) {
            fprintf(stderr, "[%s][%d] Could not read from lower layer!\n", __func__, __LINE__);
            return false;
        } else if (0 < byteCount) {
            mDecoder.feed(mpRxBuffer, byteCount);
        }

        return true;
    }

    inline bool proceedTx() {
        std::vector<std::unique_ptr<Packet>> pTxPackets;
        if (!mTxQueue.dequeue(pTxPackets) || (0 >= pTxPackets.size())) {
            // Tx queue is empty!
            return true;
        }

        if (!checkTxPipe()) {
#ifdef DEBUG
            printf("[%s][%d] Tx pipe is closed!\n", __func__, __LINE__);
#endif  // DEBUG
            return true;
        }

        ssize_t byteCount;
        for (auto& pPacket : pTxPackets) {
            std::lock_guard<std::mutex> lock(mTxMutex);
            byteCount = lwrite(pPacket);
            if (0 > byteCount) {
                fprintf(stderr, "[%s][%d] Could not write to lower layer!\n", __func__, __LINE__);
                return false;
            }
        }

        return true;
    }

    virtual ssize_t lread(const std::unique_ptr<uint8[]>&, const size_t&) = 0;
    virtual ssize_t lwrite(const std::unique_ptr<const Packet>&) = 0;

    std::mutex mRxMutex;
    std::mutex mTxMutex;

 private:
    std::unique_ptr<uint8_t[]> mpRxBuffer;
    Decoder mDecoder;

    dstruct::SyncQueue<Packet> mTxQueue;
};  // class EndPoint

}   // namespace comm

#endif // __ENPOINT_HPP__
