#ifndef __COMM_WRAPPER_HPP__
#define __COMM_WRAPPER_HPP__

#include <cstdint>

extern "C" {

bool comm_tcp_client_init(const char * const server_addr, const uint16_t& server_port);
bool comm_tcp_server_init(const uint16_t& port);
bool comm_udp_peer_init(const uint16_t& local_port, const char * const remote_addr, const uint16_t& remote_port);

void comm_deinit();

bool comm_endpoint_ready();
bool comm_peer_connected();

bool comm_p2p_endpoint_send(const uint8_t * const buffer, const size_t& buffer_size);
size_t comm_p2p_endpoint_recv_packet(uint8_t * const buffer, const size_t& buffer_size, int64_t& timestamp_us);
size_t comm_p2p_endpoint_recv_packets(
    uint8_t * const buffer, const size_t& buffer_size,
    size_t * const packet_sizes,
    int64_t * const timestamps,
    const size_t& max_number_of_packets
);

}

#endif // __COMM_WRAPPER_HPP__
