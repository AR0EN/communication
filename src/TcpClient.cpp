#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include <memory>
#include <thread>

#include "common.hpp"
#include "Encoder.hpp"
#include "Message.hpp"
#include "TcpClient.hpp"

comm::csize_t comm::TcpClient::send(std::shared_ptr<comm::IMessage> pMessage) {
    int errorCode = -1;

    // Check socket setup
    if (0 > mSocketFd) {
        printf("TCP connection is not ready!\n");
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

    int ret = write(mSocketFd, pEncodedData, encodedSize);

    if (0 > ret) {
        perror("write() -> failed!\n");
        ret = errorCode;
    }

    delete[] pEncodedData;

    return ret;
}

bool comm::TcpClient::subscribe(const std::shared_ptr<IObserver>& pObserver) {
    if (nullptr == pObserver) {
        return false;
    } else {
        mObservers.push_back(pObserver);
    }

    return true;
}

bool comm::TcpClient::start() {
    // Check socket setup
    if (0 > mSocketFd) {
        printf("TCP connection is not ready!\n");
        return false;
    }

    if (nullptr == pThread) {
        pDecoder.reset(new comm::Decoder());
        pDecoder->subscribe(shared_from_this());

        pThread.reset(new std::thread(&comm::TcpClient::run, this));
        return true;
    }

    return false;
}

void comm::TcpClient::run() {
    // Check socket setup
    if (0 > mSocketFd) {
        printf("TCP connection is not ready!\n");
        return;
    }

    int ret;
    while(!mExitFlag) {
        ret = read(mSocketFd, mRxBuffer, sizeof(mRxBuffer));
        if (0 < ret) {
            pDecoder->feed(mRxBuffer, ret);
        } else if ((0 > ret) && (EWOULDBLOCK != errno)) {
            printf("Error code: %d\n", errno);
            perror("read() -> failed!\n");
            break;
        } else {
            // Rx timeout - Do nothing
        }
    }
}
