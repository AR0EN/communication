#include <stdio.h>
#include <stdlib.h>

#include <memory>

#include "test_vectors.hpp"

#include "Encoder.hpp"

class Observer : public comm::DecodingObserver {
    public:
    int getId() {
        return 3188;
    }

    void onComplete(const std::shared_ptr<uint8_t>& pData, int size) {
        if (pData) {
            uint8_t * tmp = pData.get();

            printf("Decoded:\n\t");
            for (int i = 0; i < size; i++) {
                printf("0x%02X ", (int)tmp[i] & 0x000000FF);
            }
            printf("\n\n");
        }
    }
};

int main() {
    comm::Decoder decoder;

    std::shared_ptr<comm::DecodingObserver> pObserver = std::make_shared<Observer>();
    decoder.subscribe(pObserver);

    int i = 0;
    for (auto v : vectors) {
        decoder.feed(v, vectors_sizes[i++]);
    }

    return 0;
}
