#ifndef _OBSERVER_IMPL_HPP_
#define _OBSERVER_IMPL_HPP_

#include "Message.hpp"

class ObserverImpl : public comm::IObserver {
public:
    ObserverImpl() {}
    ~ObserverImpl() {}
    void onRecv(const std::shared_ptr<comm::Message>& pMessage) {
        if (nullptr != pMessage) {
            pMessage->dump();
        }
    }
}; // class ObserverImpl

#endif // _OBSERVER_IMPL_HPP_