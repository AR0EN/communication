#include <cstring>

#include <deque>
#include <memory>
#include <mutex>
#include <string>

#include "Packet.hpp"
#include "TcpClient.hpp"
#include "TcpServer.hpp"
#include "UdpPeer.hpp"

#include "comm_wrapper.hpp"

static std::unique_ptr<comm::P2P_Endpoint> p_endpoint;
static std::mutex endpoint_mutex;

static std::deque<std::unique_ptr<comm::Packet>> p_rx_packets;
static std::mutex rx_queue_mutex;

extern "C" {

bool comm_tcp_client_init(const char * const server_addr, const uint16_t& server_port) {
    std::lock_guard<std::mutex> lock(endpoint_mutex);
    if (nullptr != p_endpoint) {
        LOGE("[%s][%d] Endpoint was previously initialized!\n", __func__, __LINE__);
        return false;
    }

    auto server_addr_str = std::string(server_addr);
    p_endpoint = comm::TcpClient::create(server_addr_str, server_port);
    if (nullptr == p_endpoint) {
        LOGE("[%s][%d] Could not create an endpoint which connects to %s/%u!\n",
            __func__, __LINE__, server_addr, server_port
        );
        return false;
    }

    return true;
}

bool comm_tcp_server_init(const uint16_t& port) {
    std::lock_guard<std::mutex> lock(endpoint_mutex);
    if (nullptr != p_endpoint) {
        LOGE("[%s][%d] Endpoint was previously initialized!\n", __func__, __LINE__);
        return false;
    }

    p_endpoint = comm::TcpServer::create(port);
    if (nullptr == p_endpoint) {
        LOGE("[%s][%d] Could not create an endpoint which listen at port %u!\n", __func__, __LINE__, port);
        return false;
    }

    return true;
}

bool comm_udp_peer_init(const uint16_t& local_port, const char * const remote_addr, const uint16_t& remote_port) {
    std::lock_guard<std::mutex> lock(endpoint_mutex);
    if (nullptr != p_endpoint) {
        LOGE("[%s][%d] Endpoint was previously initialized!\n", __func__, __LINE__);
        return false;
    }

    auto remote_addr_str = std::string(remote_addr);
    p_endpoint = comm::UdpPeer::create(local_port, remote_addr_str, remote_port);
    if (nullptr == p_endpoint) {
        LOGE("[%s][%d] Could not create a connectionless endpoint (%u/%s/%u)!\n",
            __func__, __LINE__, local_port, remote_addr, remote_port
        );
        return false;
    }

    return true;
}

void comm_deinit() {
    std::lock_guard<std::mutex> lock(endpoint_mutex);
    if (nullptr != p_endpoint) {
        p_endpoint->close();
        p_endpoint = nullptr;
    }
}

bool comm_endpoint_ready() {
    std::lock_guard<std::mutex> lock(endpoint_mutex);
    return (nullptr != p_endpoint);
}

bool comm_peer_connected() {
    std::lock_guard<std::mutex> lock(endpoint_mutex);
    if (nullptr == p_endpoint) {
        return false;
    }

    return p_endpoint->isPeerConnected();
}

bool comm_p2p_endpoint_send(const uint8_t * const p_buffer, const size_t& buffer_size) {
    std::lock_guard<std::mutex> lock(endpoint_mutex);
    if (nullptr == p_endpoint) {
        LOGE("[%s][%d] Endpoint has not been initialized!!\n", __func__, __LINE__);
        return false;
    }

    return p_endpoint->send(comm::Packet::create(p_buffer, buffer_size));
}

size_t comm_p2p_endpoint_recv_packet(uint8_t * const p_buffer, const size_t& buffer_size, int64_t& timestamp_us) {
    std::lock_guard<std::mutex> lock(rx_queue_mutex);
    if (p_rx_packets.empty()) {
        std::lock_guard<std::mutex> lock(endpoint_mutex);
        if (nullptr != p_endpoint) {
            p_endpoint->recvAll(p_rx_packets, false);
        }
    }

    if (!p_rx_packets.empty()) {
        size_t rx_count = p_rx_packets.front()->getPayloadSize();
        if (buffer_size < rx_count) {
            LOGE("[%s][%d] Buffer size (%zu) is too small (expected: %zu)\n",
                __func__, __LINE__, buffer_size, rx_count
            );
        } else {
            memcpy(p_buffer, p_rx_packets.front()->getPayload().get(), rx_count);
            timestamp_us = p_rx_packets.front()->getTimestampUs();
            p_rx_packets.pop_front();
            return rx_count;
        }
    }

    return 0;
}

size_t comm_p2p_endpoint_recv_packets(
    uint8_t * const buffer, const size_t& buffer_size,
    size_t * const p_packet_sizes,
    int64_t * const p_timestamps,
    const size_t& max_number_of_packets
) {
    if ((0 == buffer_size) || (0 == max_number_of_packets)) {
        return 0UL;
    }

    std::lock_guard<std::mutex> lock(rx_queue_mutex);
    {
        std::lock_guard<std::mutex> lock(endpoint_mutex);
        if (nullptr != p_endpoint) {
            p_endpoint->recvAll(p_rx_packets, false);
        }
    }

#ifdef DEBUG
    if (!p_rx_packets.empty()) {
        LOGI("[%s][%d] %zu packets in Rx Queue!\n", __func__, __LINE__, p_rx_packets.size());
    }
#endif

    size_t packet_index = 0;
    size_t buffer_index = 0;
    size_t packet_size = 0;
    std::unique_ptr<comm::Packet> p_packet;
    while ((!p_rx_packets.empty()) && (max_number_of_packets > packet_index)) {
        packet_size = p_rx_packets.front()->getPayloadSize();
        if (buffer_size < (buffer_index + packet_size)) {
            break;
        }

        p_packet = std::move(p_rx_packets.front());
        p_rx_packets.pop_front();

        memcpy((buffer + buffer_index), p_packet->getPayload().get(), packet_size);
        p_timestamps[packet_index] = p_packet->getTimestampUs();
        LOGD("[%s][%d] Packet %zu (%zu bytes) at %ld (us) -> Buffer index: %zu\n",
            __func__, __LINE__,
            packet_index, packet_size, p_packet->getTimestampUs(), buffer_index
        );

        p_packet_sizes[packet_index] = packet_size;
        buffer_index += packet_size;
        packet_index++;
    }

    return buffer_index;
}

}   // extern "C"
