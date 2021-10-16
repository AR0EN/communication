#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include <memory>
#include <thread>

#include "common.hpp"
#include "Encoder.hpp"
#include "Message.hpp"
#include "TcpServer.hpp"

comm::csize_t comm::TcpServer::send(comm::IMessage& message) {
    int errorCode = -1;

    // Check socket setup
    if ((0 > mSocketFd) || (0 > mRemoteSocketFd)) {
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

    int ret = write(mRemoteSocketFd, pEncodedData.get(), encodedSize);

    if (0 > ret) {
        perror("write() -> failed!\n");
        ret = errorCode;
    }

    return ret;
}

bool comm::TcpServer::subscribe(const std::shared_ptr<IObserver>& pObserver) {
    if (pObserver) {
        mObservers.push_back(pObserver);
    } else {
        return false;
    }

    return true;
}

bool comm::TcpServer::start() {
    // Check socket setup
    if (0 > mSocketFd) {
        printf("TCP connection is not ready!\n");
        return false;
    }

    if (!pThread) {
        pDecoder.reset(new comm::Decoder());
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
