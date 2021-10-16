#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include <memory>
#include <thread>

#include "common.hpp"
#include "Encoder.hpp"
#include "Message.hpp"
#include "TcpClient.hpp"

comm::csize_t comm::TcpClient::send(comm::IMessage& message) {
    int errorCode = -1;

    // Check socket setup
    if (0 > mSocketFd) {
        printf("TCP connection is not ready!\n");
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

    int ret = write(mSocketFd, pEncodedData.get(), encodedSize);

    if (0 > ret) {
        perror("write() -> failed!\n");
        ret = errorCode;
    }

    return ret;
}

bool comm::TcpClient::subscribe(const std::shared_ptr<IObserver>& pObserver) {
    if (pObserver) {
        mObservers.push_back(pObserver);
    } else {
        return false;
    }

    return true;
}

bool comm::TcpClient::start() {
    // Check socket setup
    if (0 > mSocketFd) {
        printf("TCP connection is not ready!\n");
        return false;
    }

    if (!pThread) {
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
