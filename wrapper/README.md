# A wrapper for Communication Library
* `bool comm_tcp_client_init(const char * const server_addr, const uint16_t& server_port)`
  * Initialize Tcp Client Endpoint
  * Parameters
    * `server_addr`: [in] IP address of TCP Server
    * `server_port`: [in] TCP Port at which Server is listening

* `bool comm_tcp_server_init(const uint16_t& port)`
  * Initialize Tcp Server Endpoint
  * Parameters
    * `port`: [in] TCP Port at which Server will listening

* `bool comm_udp_peer_init(...)`
  * Initialize Tcp Server Endpoint
  * Parameters
    * `local_port` : [in] TCP Port at which the endpoint will listening
    * `remote_addr`: [in] IP address of UDP Peer
    * `remote_port`: [in] TCP Port at which UDP Peer is listening

* `void comm_deinit()`: release resources

* `bool comm_endpoint_ready()`: return true if the endpoint has been initialized successfully
* `bool comm_peer_connected()`: return true if Peer Connection has been established successfully
  * Note: Tcp Client/UDP Peer endpoints will always return `true`

* `bool comm_p2p_endpoint_send(...)`
  * Send data to Peer Endpoint.
  * Return `true` if data has been enqueued successfully
  * Parameters
    * `buffer`     : [in] pointer which points to the location of Tx data
    * `buffer_size`: [in] size of Tx data

* `size_t comm_p2p_endpoint_recv_packet(uint8_t * const p_buffer, const size_t& buffer_size, int64_t& timestamp_us)`
  * Try to read a single packet (if available) from Rx Pipe
  * Return number of bytes read from Rx Pipe
  * Parameters
  * `p_buffer`    : [in] pointer which points to the location of Rx buffer
  * `buffer_size` : [in] size of Rx buffer
    Make sure Rx buffer is big enough to contain at least one packet!

  * `timestamp_us`: [out] timestamp in microseconds at which the packet was received (decoded)

* `size_t comm_p2p_endpoint_recv_packets()`
  * Try to read multiple packets (if available) from Rx Pipe
  * Return number of bytes read from Rx Pipe
  * Parameters
    * `p_buffer`             : [in] pointer which points to the location of Rx buffer
    * `buffer_size`          : [in] size of Rx buffer
      Make sure Rx buffer is big enough to contain at least one packet!

    * `p_packet_sizes`       : [in/out] pointer which points to the beginning of an array of `size_t`.
      Wrapper library will update the array 's elements with the sizes of received packets

    * `p_timestamps`         : [in/out] pointer which points to the beginning of the array of `int64_t`.
      Wrapper library will update the array 's elements with the timestamps (us) of received packets

    * `max_number_of_packets`: [in] maximum number of packets which the application could handle at a time
