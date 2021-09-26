#ifndef _CONNECTIONLESS_PEER_HPP_
#define _CONNECTIONLESS_PEER_HPP_

#include <stdint.h>

#include "common.hpp"
#include "Message.hpp"

namespace comm {

class ConnectionlessPeer {
public:
    virtual ~ConnectionlessPeer() {}
    virtual csize_t send(const char * ipAddress, uint16_t port, Message msg) = 0;
    virtual bool subscribe(std::shared_ptr<Observer>& pObserver) = 0;
    // virtual void unsubscribe() = 0;
};

}; // namespace comm

#endif // _CONNECTIONLESS_PEER_HPP_
