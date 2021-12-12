#include "EndPoint.hpp"

namespace comm {

inline bool EndPoint::send(std::unique_ptr<Packet>& pPacket) {
    if (pPacket) {
        mTxQueue.enqueue(pPacket);
        return true;
    } else {
        LOGE("[%s][%d] Tx packet must not be empty!\n", __func__, __LINE__);
        return false;
    }
}

inline bool EndPoint::send(std::unique_ptr<Packet>&& pPacket) {
    if (pPacket) {
        mTxQueue.enqueue(pPacket);
        return true;
    } else {
        LOGE("[%s][%d] Tx packet must not be empty!\n", __func__, __LINE__);
        return false;
    }
}

inline bool EndPoint::checkRxPipe() {
    LOGD("[%s][%d]\n", __func__, __LINE__);
    return true;
}

inline bool EndPoint::checkTxPipe() {
    LOGD("[%s][%d]\n", __func__, __LINE__);
    return true;
}

inline bool EndPoint::recvAll(std::vector<std::unique_ptr<Packet>>& pRxPackets) {
    return mDecoder.dequeue(pRxPackets);
}

}   // namespace comm
