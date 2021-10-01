#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include <memory>
#include <thread>

#include "common.hpp"
#include "Encoder.hpp"
#include "Message.hpp"
#include "TcpServer.hpp"

comm::csize_t comm::TcpServer::send(std::shared_ptr<comm::IMessage> pMessage) {
    int errorCode = -1;

    // Check socket setup
    if ((0 > mSocketFd) || (0 > mRemoteSocketFd)) {
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

    int ret = write(mRemoteSocketFd, pEncodedData, encodedSize);

    if (0 > ret) {
        perror("write() -> failed!\n");
        ret = errorCode;
    }

    delete[] pEncodedData;

    return ret;
}

bool comm::TcpServer::subscribe(const std::shared_ptr<IObserver>& pObserver) {
    if (nullptr == pObserver) {
        return false;
    } else {
        mObservers.push_back(pObserver);
    }

    return true;
}

bool comm::TcpServer::start() {
    // Check socket setup
    if (0 > mSocketFd) {
        printf("TCP connection is not ready!\n");
        return false;
    }

    if (nullptr == pThread) {
        pDecoder = std::make_shared<comm::Decoder>();
        pDecoder->subscribe(shared_from_this());

        pThread.reset(new std::thread(&comm::TcpServer::run, this));
        return true;
    }

    return false;
}

void comm::TcpServer::run() {
    // Check socket setup
    if (0 > mSocketFd) {
        printf("TCP connection is not ready!\n");
        return;
    }

    struct sockaddr_in remoteSocketAddr;
    socklen_t remoteAddressSize = (socklen_t)sizeof(remoteSocketAddr);

    while(!mExitFlag) {
        mRemoteSocketFd = accept(mSocketFd, (struct sockaddr*)&remoteSocketAddr, &remoteAddressSize);
        if (0 <= mRemoteSocketFd) {
            receive();
        } else if ((mRemoteSocketFd < 0) && (EAGAIN != errno)) {
            printf("Error code: %d\n", errno);
            perror("accept() -> failed!\n");
            return;
        } else {
            // Timeout - do nothing
        }

        close(mRemoteSocketFd);
        mRemoteSocketFd = -1;
    }
}

void comm::TcpServer::receive() {
    if ((0 > mSocketFd) || (0 > mRemoteSocketFd)) {
        return;
    }

    int ret;
    while(!mExitFlag) {
        ret = read(mRemoteSocketFd, mRxBuffer, sizeof(mRxBuffer));
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
