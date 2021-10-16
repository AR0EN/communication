#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include <memory>
#include <thread>

#include "common.hpp"
#include "Encoder.hpp"
#include "Message.hpp"
#include "UdpPeer.hpp"

comm::csize_t comm::UdpPeer::send(
    const char * ipAddress, const uint16_t& port, comm::IMessage& message) {

    int errorCode = -1;

    // Check socket setup
    if (0 > mSocketFd) {
        printf("UDP socket has not been setup!\n");
        return errorCode;
    }
    errorCode--;

    // Prepare data for transmission
    std::unique_ptr<uint8_t[]> pSerializedData;
    csize_t serializedSize;

    message.serialize(pSerializedData, serializedSize);

    if (!pSerializedData) {
        return errorCode;
    }
    errorCode--;

    std::unique_ptr<uint8_t[]> pEncodedData;
    csize_t encodedSize;

    comm::encode(pSerializedData, serializedSize, pEncodedData, encodedSize);

    if (!pEncodedData) {
        return errorCode;
    }
    errorCode--;

    // Destination address
    struct sockaddr_in peerAddr;
    peerAddr.sin_family      = AF_INET;
    peerAddr.sin_port        = htons(port);
    peerAddr.sin_addr.s_addr = inet_addr(ipAddress);

    // Send data over UDP
    int ret = sendto( \
                mSocketFd, \
                pEncodedData.get(), encodedSize, 0, \
                (struct sockaddr *)&peerAddr, sizeof(peerAddr));

    if (0 > ret) {
        perror("Could not send data!\n");
        ret = errorCode;
    }

    return ret;
}

bool comm::UdpPeer::subscribe(const std::shared_ptr<IObserver>& pObserver) {
    if (pObserver) {
        mObservers.push_back(pObserver);
    } else {
        return false;
    }

    return true;
}

bool comm::UdpPeer::start() {
    // Check socket setup
    if (0 > mSocketFd) {
        printf("UDP socket has not been setup!\n");
        return false;
    }

    if (!pThread) {
        pDecoder.reset(new comm::Decoder());
        pDecoder->subscribe(shared_from_this());

        pThread.reset(new std::thread(&comm::UdpPeer::run, this));
        return true;
    }

    return false;
}

void comm::UdpPeer::run() {
    // Check socket setup
    if (0 > mSocketFd) {
        printf("UDP socket has not been setup!\n");
        return;
    }

    socklen_t remoteAddressSize;
    struct sockaddr_in remoteSocketAddr;
    remoteAddressSize = (socklen_t)sizeof(remoteSocketAddr);

    int ret;
    while(!mExitFlag) {
        ret = recvfrom(mSocketFd, mRxBuffer, sizeof(mRxBuffer), 0, (struct sockaddr *) &remoteSocketAddr, &remoteAddressSize);

        if (0 < ret) {
            pDecoder->feed(mRxBuffer, ret);
        } else if ((0 > ret) && (EWOULDBLOCK != errno)) {
            printf("Error code: %d\n", errno);
            perror("recvfrom() -> failed!\n");
            break;
        } else {
            // Rx timeout - Do nothing
        }
    }
}
