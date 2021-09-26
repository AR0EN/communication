#ifndef _OBSERVER_HPP_
#define _OBSERVER_HPP_

#include <memory>

#include "Message.hpp"

namespace comm {

class NetObserver {
public:
    virtual ~NetObserver() {}
    virtual void onRecv(const std::shared_ptr<NetMessage>& pMessage) = 0;
}; // class NetObserver

}; // namespace comm

#endif // _OBSERVER_HPP_
