#ifndef __COMM_WRAPPER_HPP__
#define __COMM_WRAPPER_HPP__

#include <cstdint>

extern "C" {

bool comm_tcp_client_init(const char * const server_addr, const uint16_t& server_port);
bool comm_tcp_server_init(const uint16_t& port);
bool comm_udp_peer_init(const uint16_t& local_port, const char * const remote_addr, const uint16_t& remote_port);

void comm_deinit();

bool comm_p2p_endpoint_send(const uint8_t * const buffer, const size_t& buffer_size);
ssize_t comm_p2p_endpoint_recv(uint8_t * const buffer, const size_t& buffer_size);

}

#endif // __COMM_WRAPPER_HPP__
