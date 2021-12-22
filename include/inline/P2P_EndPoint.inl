#include "P2P_EndPoint.hpp"

namespace comm {

inline bool P2P_EndPoint::send(std::unique_ptr<Packet>& pPacket) {
    if (pPacket) {
        mTxQueue.enqueue(pPacket);
        return true;
    } else {
        LOGE("[%s][%d] Tx packet must not be empty!\n", __func__, __LINE__);
        return false;
    }
}

inline bool P2P_EndPoint::send(std::unique_ptr<Packet>&& pPacket) {
    if (pPacket) {
        mTxQueue.enqueue(pPacket);
        return true;
    } else {
        LOGE("[%s][%d] Tx packet must not be empty!\n", __func__, __LINE__);
        return false;
    }
}

inline bool P2P_EndPoint::recvAll(std::deque<std::unique_ptr<Packet>>& pRxPackets, bool wait) {
    return mDecoder.dequeue(pRxPackets, wait);
}

}   // namespace comm
