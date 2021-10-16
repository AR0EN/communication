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
    const char * ipAddress, uint16_t port, std::shared_ptr<comm::IMessage> pMessage) {

    int errorCode = -1;

    // Check socket setup
    if (0 > mSocketFd) {
        printf("UDP socket has not been setup!\n");
        return errorCode;
    }
    errorCode--;

    // Prepare data for transmission
    if (nullptr == pMessage) {
        return errorCode;
    }
    errorCode--;

    uint8_t * pSerializedData;
    int serializedSize;

    pMessage->serialize(pSerializedData, serializedSize);

    if (nullptr == pSerializedData) {
        return errorCode;
    }
    errorCode--;

    uint8_t * pEncodedData;
    int encodedSize;

    comm::encode(pSerializedData, serializedSize, pEncodedData, encodedSize);

    delete[] pSerializedData;
    if (nullptr == pEncodedData) {
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
                pEncodedData, encodedSize, 0, \
                (struct sockaddr *)&peerAddr, sizeof(peerAddr));

    if (0 > ret) {
        perror("Could not send data!\n");
        ret = errorCode;
    }

    delete[] pEncodedData;

    return ret;
}

bool comm::UdpPeer::subscribe(const std::shared_ptr<IObserver>& pObserver) {
    if (nullptr == pObserver) {
        return false;
    } else {
        mObservers.push_back(pObserver);
    }

    return true;
}

bool comm::UdpPeer::start() {
    // Check socket setup
    if (0 > mSocketFd) {
        printf("UDP socket has not been setup!\n");
        return false;
    }

    if (nullptr == pThread) {
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
