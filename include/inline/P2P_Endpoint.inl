#include "P2P_Endpoint.hpp"

namespace comm {

inline bool P2P_Endpoint::send(std::unique_ptr<Packet>& pPacket) {
    if (pPacket) {
        if (!mTxQueue.enqueue(pPacket)) {
            LOGE("Tx Queue is full!\n");
        }
        return true;
    } else {
        LOGE("[%s][%d] Tx packet must not be empty!\n", __func__, __LINE__);
        return false;
    }
}

inline bool P2P_Endpoint::send(std::unique_ptr<Packet>&& pPacket) {
    if (pPacket) {
        if (!mTxQueue.enqueue(pPacket)) {
            LOGE("Tx Queue is full!\n");
        }
        return true;
    } else {
        LOGE("[%s][%d] Tx packet must not be empty!\n", __func__, __LINE__);
        return false;
    }
}

inline bool P2P_Endpoint::recvAll(std::deque<std::unique_ptr<Packet>>& pRxPackets, bool wait) {
    return mDecoder.dequeue(pRxPackets, wait);
}

inline bool P2P_Endpoint::isPeerConnected() {
    return true;
}

}   // namespace comm
