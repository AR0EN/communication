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

class Enpoint {
 public:
    virtual ~Enpoint() {}

    virtual bool send(std::unique_ptr<Packet>& pPacket) {
        if (pPacket) {
            mTxQueue.enqueue(pPacket);
            return true;
        } else {
            fprintf(stderr, "[%s][%d] Tx packet must not be empty!\n", __func__, __LINE__);
            return false;
        }
    }

    virtual bool consumeRx(std::vector<std::unique_ptr<Packet>>& pRxPackets) {
        return mDecoder.dequeue(pRxPackets);
    }

 protected:
    Enpoint() : mRxFileDescriptor(-1), mTxFileDescriptor(-1) {
        mpRxBuffer.reset(new uint8_t[MAX_FRAME_SIZE]);
    }

    inline virtual bool proceedRx() {
        ssize_t byteCount = 0;

        {
            std::lock_guard<std::mutex> lock(mRxMutex);

            if (0 >= mRxFileDescriptor) {
#ifdef DEBUG
                fprintf(stderr, "[%s][%d] Rx descriptor (%d) is invalid!\n",
                    __func__, __LINE__, mRxFileDescriptor
                );
#endif  // DEBUG
                return true;
            }

            byteCount = read(mRxFileDescriptor, mpRxBuffer.get(), MAX_FRAME_SIZE);
        }

        if ((0 > byteCount) && (EWOULDBLOCK != errno)) {
            fprintf(stderr, "[%s][%d] Could not read from lower layer!\n", __func__, __LINE__);
            perror(NULL);
            return false;
        } else {
            mDecoder.feed(mpRxBuffer, byteCount);
        }

        return true;
    }

    inline virtual bool proceedTx() {
        std::vector<std::unique_ptr<Packet>> pTxPackets;
        if (!mTxQueue.dequeue(pTxPackets) || (0 >= pTxPackets.size())) {
            // Tx queue is empty!
            return true;
        }

        if (0 >= mTxFileDescriptor) {
#ifdef DEBUG
            printf("[%s][%d] Tx descriptor (%d) is invalid!\n",
                __func__, __LINE__, mTxFileDescriptor
            );
#endif  // DEBUG
            return true;
        }

        ssize_t byteCount;
        for (auto& pPacket : pTxPackets) {
            std::lock_guard<std::mutex> lock(mTxMutex);
            if (0 < mTxFileDescriptor) {
                byteCount = write(mTxFileDescriptor, pPacket->getPayload().get(), pPacket->getPayloadSize());
                if (0 > byteCount) {
                    fprintf(stderr, "[%s][%d] Could not write to lower layer!\n", __func__, __LINE__);
                    perror(NULL);
                    return false;
                }
            } else {
                // Tx pipe has been closed!
                break;
            }
        }

        return true;
    }

    inline void setRxFileDescriptor(int rxFileDescriptor) {
        std::lock_guard<std::mutex> lock(mRxMutex);
        mRxFileDescriptor = rxFileDescriptor;
    }

    inline void setTxFileDescriptor(int txFileDescriptor) {
        std::lock_guard<std::mutex> lock(mTxMutex);
        mTxFileDescriptor = txFileDescriptor;
    }

    std::mutex mRxMutex;
    std::mutex mTxMutex;

 private:
    int mRxFileDescriptor;
    std::unique_ptr<uint8_t[]> mpRxBuffer;

    Decoder mDecoder;

    int mTxFileDescriptor;
    dstruct::SyncQueue<Packet> mTxQueue;
};  // class Enpoint

}   // namespace comm

#endif // __ENPOINT_HPP__
