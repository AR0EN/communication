#ifndef __COMM_WRAPPER_HPP__
#define __COMM_WRAPPER_HPP__

#include <cstdint>

#include "common.hpp"

extern "C" {

bool comm_tcp_client_init(const char * const server_addr, const uint16_t& server_port);
bool comm_tcp_server_init(const uint16_t& port);
bool comm_udp_peer_init(const uint16_t& local_port, const char * const remote_addr, const uint16_t& remote_port);

void comm_deinit();

bool comm_endpoint_ready();
bool comm_peer_connected();

bool comm_p2p_endpoint_send(const uint8_t * const buffer, const size_t& buffer_size);
ssize_t comm_p2p_endpoint_recv_packet(uint8_t * const buffer, const size_t& buffer_size, int64_t& timestamp_us);

/**
 * Format: ... | Timestamp in us (int64_t LE) of packet n | Size of packet n (uint32_t LE) | Packet n | ...
 */
constexpr size_t PACKET_TIMESTAMP_SIZE = 8UL;
constexpr size_t SIZE_OF_PACKET_SIZE   = 4UL;
ssize_t comm_p2p_endpoint_recv_packets(uint8_t * const buffer, const size_t& buffer_size);


}

#endif // __COMM_WRAPPER_HPP__
