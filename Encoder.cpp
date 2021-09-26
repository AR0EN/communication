#include <stdint.h>

#include <memory>

#include "Encoder.hpp"

void comm::Decoder::feed(uint8_t * pData, csize_t size) {
    for (int i = 0; i < size; i++) {
        proceed(pData[i]);
    }
}

void comm::Decoder::subscribe(std::shared_ptr<comm::DecodingObserver>& pObserver) {
    if (nullptr != pObserver) {
        mObservers.push_back(pObserver);
    }
}

void comm::Decoder::notify() {
    if (nullptr != pPayload) {
        std::shared_ptr<uint8_t> payload(new uint8_t[mPayloadSize], std::default_delete<uint8_t[]> ());
        memcpy(payload.get(), pPayload, mPayloadSize);
        for (auto pObserver : mObservers) {
            if (pObserver) {
                pObserver->onComplete(payload, mPayloadSize);
            }
        }
    }
}
