# Communication Library
A lightweight library for Packet-based P2P Communication

## Overview

<p align="center">
  <img src="./docs/overview.jpg" width="480" />
</p>

## Directory Structure
```
.
├── docs/
├── include/
│   └── inline
├── src/
├── test/
├── tools/
├── wrapper
│   └── README.md
├── CMakeLists.txt
├── LICENSE
├── README.md
└── setup.sh
```

* `docs`    : documentation
* `include` : libcomm headers
* `src`     : libcomm implementation
* `test`    : unit tests (native & python)
* `tool`    : scripts for generating data for unit testing
* `wrapper` : wrapper for libcomm

## Usage
* `UdpPeer`
```
// Construct an UdpPeer
std::unique_ptr<comm::P2P_Endpoint> pEndpoint =
    comm::UdpPeer::create(<Local Port>, <Peer/Remote IP Address>, <Peer/Remote Port>);
...

// Send a packet to Peer
udpPeer.send(comm::Packet::create(<buffer address>, <buffer size>));
...

// Check Rx queue
std::deque<std::unique_ptr<comm::Packet>> pPackets;
if (udpPeer.recvAll(pPackets)) {
    for (auto& pPacket : pPackets) {
        // do something
    }
}
...
```

* TCP Server
```
// Construct a Tcp Server
std::unique_ptr<comm::P2P_Endpoint> pTcpServer = comm::TcpServer::create(<Local Port>);
...

// Waiting for client
while (!pTcpServer->isPeerConnected()) {}
...

// Send a packet to Tcp Client
pTcpServer->send(comm::Packet::create(<buffer address>, <buffer size>));
...

// Check Rx queue
std::deque<std::unique_ptr<comm::Packet>> pPackets;
if (pTcpServer->recvAll(pPackets)) {
    for (auto& pPacket : pPackets) {
        // do something
    }
}
...
```

* TCP Client
```
// Construct a Tcp Client
std::unique_ptr<comm::P2P_Endpoint> pTcpClient =
    comm::TcpClient::create(<Server IP Address>, <Server Port>);

if (!pTcpClient) {
    LOGE("Could not create Tcp Client which connects to %s/%u!\n", <Server IP Address>, <Server Port>);
    return 1;
}
...

// Send a packet to Tcp Server
pTcpClient->send(comm::Packet::create(<buffer address>, <buffer size>));
...

// Check Rx queue
std::deque<std::unique_ptr<comm::Packet>> pPackets;
if (pTcpClient->recvAll(pPackets)) {
    for (auto& pPacket : pPackets) {
        // do something
    }
}
...
```

## Compilation
* Ubuntu
```
rm -rf build-linux
mkdir build-linux
cd build-linux
cmake ..
make
```

* Windows (MinGW64)
```
rm -rf build-mingw
mkdir build-mingw
cd build-mingw
cmake -G "MinGW Makefiles" ..
cmake --build .
```

* Note
  * CMAKE Option `-DDEFINE_DEBUG=ON`: to enable debug log
  * CMAKE Option `-DDEFINE_PROFILING=ON`: to enable profiling
  * CMAKE Option `-DBUILD_TESTS=OFF`: to disable unit tests' compilation
  * CMAKE Option `-DUSE_RAW_POINTER=ON`: to use Raw Pointers in unit tests
