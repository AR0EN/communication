#include "P2P_EndPoint.hpp"

namespace comm {

bool P2P_EndPoint::proceedRx() {
    std::lock_guard<std::mutex> lock(mRxMutex);

    ssize_t byteCount = lread(mpRxBuffer, MAX_FRAME_SIZE);

    if (0 > byteCount) {
        LOGE("[%s][%d] Could not read from lower layer!\n", __func__, __LINE__);
        return false;
    } else if (0 < byteCount) {
        mDecoder.feed(mpRxBuffer, byteCount);
    } else {
        // Do nothing
    }

    return true;
}

bool P2P_EndPoint::proceedTx() {
    std::lock_guard<std::mutex> lock(mTxMutex);

    std::deque<std::unique_ptr<Packet>> pTxPackets;
    if (!mTxQueue.dequeue(pTxPackets) || (0 >= pTxPackets.size())) {
        // Tx queue is empty!
        return true;
    }

    LOGD("[%s][%d] %lu packets in Tx queue\n", __func__, __LINE__, pTxPackets.size());

    std::unique_ptr<uint8_t[]> pEncodedData;
    size_t encodedSize;
    ssize_t byteCount;

    for (auto& pPacket : pTxPackets) {
        encode(pPacket->getPayload(), pPacket->getPayloadSize(), mTransactionId++,
            pEncodedData, encodedSize
        );

        if ((!pEncodedData) || (0 == encodedSize)) {
            LOGE("[%s][%d] Could not encode data!\n", __func__, __LINE__);
            continue;
        }

        byteCount = lwrite(pEncodedData, encodedSize);
        if (0 > byteCount) {
            LOGE("[%s][%d] Could not write to lower layer!\n", __func__, __LINE__);
            return false;
        } else {
            LOGD("[%s][%d] Wrote %ld bytes\n", __func__, __LINE__, byteCount);
        }
    }

    return true;
}

}   // namespace comm
