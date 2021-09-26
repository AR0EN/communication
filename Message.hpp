#ifndef _MESSAGE_HPP_
#define _MESSAGE_HPP_

#include <stdint.h>

#include <memory>

#include "common.hpp"

namespace comm {

class Message {
public:
    virtual ~Message() {}
    virtual bool serialize(uint8_t *& pSerializedData, int& serializedSize) = 0;
    virtual bool deserialize(uint8_t * pSerializedData, int serializedSize) = 0;
}; // class Message

class Observer {
public:
    virtual ~Observer() {}
    virtual void onRecv(std::shared_ptr<Message>& pMessage) = 0;
}; // class Observer


}; // namespace comm

#endif // _MESSAGE_HPP_
