#ifndef __ENPOINT_HPP__
#define __ENPOINT_HPP__

#include <errno.h>
#include <cstdint>
#include <cstdio>
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

    bool send(std::unique_ptr<Packet>&& pPacket) {
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
        printf("[%s][%d]\n", __func__, __LINE__);
        return true;
    }

    virtual bool checkTxPipe() {
        printf("[%s][%d]\n", __func__, __LINE__);
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

#ifdef DEBUG
        printf("[%s][%d] %lu packets in Tx queue\n", __func__, __LINE__, pTxPackets.size());
#endif  // DEBUG

        if (!checkTxPipe()) {
#ifdef DEBUG
            printf("[%s][%d] Tx pipe is closed!\n", __func__, __LINE__);
#endif  // DEBUG
            return true;
        }

        std::unique_ptr<uint8_t[]> pEncodedData;
        size_t encodedSize;
        ssize_t byteCount;
        for (auto& pPacket : pTxPackets) {
            comm::encode(pPacket->getPayload(), pPacket->getPayloadSize(),
                pEncodedData, encodedSize
            );

            if ((!pEncodedData) || (0 == encodedSize)) {
                fprintf(stderr, "[%s][%d] Could not encode data!\n", __func__, __LINE__);
                continue;
            }

            std::lock_guard<std::mutex> lock(mTxMutex);
            byteCount = lwrite(pEncodedData, encodedSize);
            if (0 > byteCount) {
                fprintf(stderr, "[%s][%d] Could not write to lower layer!\n", __func__, __LINE__);
                return false;
            } else {
#ifdef DEBUG
                printf("[%s][%d] Wrote %ld bytes\n", __func__, __LINE__, byteCount);
#endif  // DEBUG
            }
        }

        return true;
    }

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

#endif // __ENPOINT_HPP__
