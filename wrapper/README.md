# A wrapper for Communication Library
## Python Wrapper
`comm_wrapper.py` provides an easy to use Python API on top of Communication Library (libcomm)

* Dependency Graph
```
Python Application
  --> comm_wrapper.py
    --> libcommw (shared)
      --> libcomm (static)
```


* Guidelines
  * Step 1: load `libcommw` library.
    ```
    import comm_wrapper
    comm_wrapper.load_library(config_path='config.ini')
    ```
    * `comm_wrapper.load_library()` requires an input, which is the path to the configuration file
      * Default configuration path: `./config.ini`
      * Configuration file needs to provide the following information (refer to `<REPO>/test/config.ini`)
        * `[comm-wrapper]`   : main section
        * `mingw64_bin_dir`  : path to `mingw64/bin` (only required on Windows!)
        * `wrapper_lib_mingw`: path to `libcommw.dll` (only required on Windows!)
        * `wrapper_lib_linux`: path to `libcommw.so` (only required on Linux!)


  * Step 2: initialize Communication Endpoint. Supported Endpoints
    * TCP Client -> `comm_tcp_client_init(server_addr, server_port)`
    * TCP Server -> `comm_tcp_server_init(port)`
    * UDP Peer   -> `comm_udp_peer_init(local_port, remote_addr, remote_port)`


  * Step 3: check if Endpoint has been initialized successfully
    ```
    if (not comm_wrapper.comm_endpoint_ready()):
        print('Endpoint is not ready!')
        exit(1)
    ```


  * Step 4: wait until connection with Peer is established
    ```
    import time
    t0 = time.monotonic()
    while ((TIMEOUT_S > (time.monotonic() - t0)) and (not comm_wrapper.comm_peer_connected())):
        pass
    ```


  * Step 5: exchange data with Peer
    * Send a packet to Peer: `comm_p2p_endpoint_send(buffer)`

    * Read a single packet (if available) from Rx Pipe: `comm_p2p_endpoint_recv_packet()`
      * The function will return a dictionary (empty if Rx Pipe is empty)
        ```
        {
          "data": <received data - a `Bytes` object)>
          "timestamp_us": <timestamp in microseconds, at which the packet was received (decoded) - an integer>
        }
        ```

    * Read multiple packets from (if available) from Rx Pipe: `comm_p2p_endpoint_recv_packets()`
      * The function will return a list of dictionaries. Each element will represent a received packet


  * Step 6: release resources used by `libcomm`
    ```
    comm_wrapper.comm_deinit()
    ```

* Sample applications
  * `<REPO>/test/ut_tcp_client.py`: a TCP Client
  * `<REPO>/test/ut_tcp_server.py`: a TCP Server
  * `<REPO>/test/ut_udp_peer.py`  : an UDP Peer
  * `<REPO>/test/ut_tcpip_ep.py`  : common functions used by all endpoints

---
## Native Wrapper
* `bool comm_tcp_client_init(const char * const server_addr, const uint16_t& server_port)`
  * Initialize TCP Client Endpoint
  * Parameters
    * `server_addr`: [in] IP address of TCP Server
    * `server_port`: [in] TCP Port at which Server is listening


* `bool comm_tcp_server_init(const uint16_t& port)`
  * Initialize TCP Server Endpoint
  * Parameters
    * `port`: [in] TCP Port at which Server will listening


* `bool comm_udp_peer_init(...)`
  * Initialize UDP Peer Endpoint
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
