#include "EndPoint.hpp"

namespace comm {

bool EndPoint::proceedRx() {
    std::lock_guard<std::mutex> lock(mRxMutex);

    if (!checkRxPipe()) {
        LOGD("[%s][%d] Rx pipe is closed!\n", __func__, __LINE__);
        return true;
    }

    ssize_t byteCount = lread(mpRxBuffer, MAX_FRAME_SIZE);

    if (0 > byteCount) {
        LOGE("[%s][%d] Could not read from lower layer!\n", __func__, __LINE__);
        return false;
    } else if (0 < byteCount) {
        mDecoder.feed(mpRxBuffer, byteCount);
    }

    return true;
}

bool EndPoint::proceedTx() {
    std::lock_guard<std::mutex> lock(mTxMutex);

    std::vector<std::unique_ptr<Packet>> pTxPackets;
    if (!mTxQueue.dequeue(pTxPackets) || (0 >= pTxPackets.size())) {
        // Tx queue is empty!
        return true;
    }

    LOGD("[%s][%d] %lu packets in Tx queue\n", __func__, __LINE__, pTxPackets.size());

    if (!checkTxPipe()) {
        LOGD("[%s][%d] Tx pipe is closed!\n", __func__, __LINE__);
        return true;
    }

    std::unique_ptr<uint8_t[]> pEncodedData;
    size_t encodedSize;
    ssize_t byteCount;
    for (auto& pPacket : pTxPackets) {
        encode(pPacket->getPayload(), pPacket->getPayloadSize(),
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
