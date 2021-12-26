import configparser
import ctypes
import os
import sys

wrapper = None

# bool comm_tcp_client_init(const char * const server_addr, const uint16_t& server_port);
def comm_tcp_client_init(server_addr, server_port):
    global wrapper
    if (not wrapper):
        print('Shared library must be loaded in advance!')
        return False

    if (not server_addr) or (type(server_addr) is not str):
        print('1st argument, address of Tcp Server, must be a string!')
        return False

    if (not server_port) or (type(server_port) is not int) or (0 >= server_port):
        print('2nd argument, Tcp Server Port, must be positive integer!')
        return False

    wrapper.comm_tcp_client_init.restype = ctypes.c_bool
    cserver_addr = ctypes.c_char_p(server_addr.encode('utf-8'))
    cserver_port = ctypes.c_uint16(server_port)
    return wrapper.comm_tcp_client_init(cserver_addr, ctypes.byref(cserver_port))

# bool comm_tcp_server_init(const uint16_t& port);
def comm_tcp_server_init(port):
    global wrapper
    if (not wrapper):
        print('Shared library must be loaded in advance!')
        return False

    if (not port) or (type(port) is not int) or (0 >= port):
        print('1st argument, Tcp Server Port, must be positive integer!')
        return False

    wrapper.comm_tcp_server_init.restype = ctypes.c_bool
    cport = ctypes.c_uint16(port)
    return wrapper.comm_tcp_server_init(ctypes.byref(cport))

# bool comm_udp_peer_init(const uint16_t& local_port, const char * const remote_addr, const uint16_t& remote_port);
def comm_udp_peer_init(local_port, remote_addr, remote_port):
    global wrapper
    if (not wrapper):
        print('Shared library must be loaded in advance!')
        return False

    if (not local_port) or (type(local_port) is not int) or (0 >= local_port):
        print('1st argument, Local Port, must be positive integer!')
        return False

    if (not remote_addr) or (type(remote_addr) is not str):
        print('2nd argument, Peer Address, must be a string!')
        return False

    if (not remote_port) or (type(remote_port) is not int) or (0 >= remote_port):
        print('3rd argument, Peer Port, must be positive integer!')
        return False

    wrapper.comm_udp_peer_init.restype = ctypes.c_bool
    clocal_port = ctypes.c_uint16(local_port)
    cremote_addr = ctypes.c_char_p(remote_addr.encode('utf-8'))
    cremote_port = ctypes.c_uint16(remote_port)
    return wrapper.comm_udp_peer_init(ctypes.byref(clocal_port), cremote_addr, ctypes.byref(cremote_port))

# void comm_deinit();
def comm_deinit():
    global wrapper
    wrapper.comm_deinit()

# bool comm_endpoint_ready();
def comm_endpoint_ready():
    global wrapper
    wrapper.comm_endpoint_ready.restype = ctypes.c_bool
    return wrapper.comm_endpoint_ready()

# bool comm_peer_connected();
def comm_peer_connected():
    global wrapper
    wrapper.comm_peer_connected.restype = ctypes.c_bool
    return wrapper.comm_peer_connected()

# bool comm_p2p_endpoint_send(const uint8_t * const buffer, const size_t& buffer_size);
def comm_p2p_endpoint_send(buffer):
    global wrapper
    if (not wrapper):
        print('Shared library must be loaded in advance!')
        return False

    if (not buffer) or ((type(buffer) is not bytes) and (type(buffer) is not bytearray)):
        print('1st argument, Tx Buffer, must be an instance of `bytes`/`bytearray`!')
        return False

    wrapper.comm_p2p_endpoint_send.restype = ctypes.c_bool
    cbuffer_size = ctypes.c_size_t(len(buffer))
    if (type(buffer) is bytes):
        return wrapper.comm_p2p_endpoint_send(buffer, ctypes.byref(cbuffer_size))
    else:
        return wrapper.comm_p2p_endpoint_send(bytes(buffer), ctypes.byref(cbuffer_size))

PACKET_KEY_DATA      = 'data'
PACKET_KEY_TIMESTAMP = 'timestamp_us'
C_RX_BUFFER_SIZE = ctypes.c_size_t(4096)
CREF_RX_BUFFER_SIZE = ctypes.byref(C_RX_BUFFER_SIZE)
c_rx_buffer = (ctypes.c_uint8 * C_RX_BUFFER_SIZE.value)()

# size_t comm_p2p_endpoint_recv_packet(uint8_t * const buffer, const size_t& buffer_size, int64_t& timestamp_us);
def comm_p2p_endpoint_recv_packet():
    global wrapper
    packet = {}
    rx_buf = None
    ctimestamp_us = ctypes.c_int64(-1)
    if (not wrapper):
        print('Shared library must be loaded in advance!')
        return packet

    wrapper.comm_p2p_endpoint_recv_packet.restype = ctypes.c_size_t

    rx_byte_count = wrapper.comm_p2p_endpoint_recv_packet(c_rx_buffer, CREF_RX_BUFFER_SIZE, ctypes.byref(ctimestamp_us))
    if (0 < rx_byte_count):
        packet[PACKET_KEY_DATA] = bytes(c_rx_buffer)[:rx_byte_count]
        packet[PACKET_KEY_TIMESTAMP] = ctimestamp_us.value
    elif (0 > rx_byte_count):
        # Should not happen!
        print('Something was wrong (error code: {})!'.format(rx_byte_count))

    return packet

# size_t comm_p2p_endpoint_recv_packets(
#     uint8_t * const buffer, const size_t& buffer_size,
#     size_t * const packet_sizes,
#     int64_t * const timestamps,
#     const size_t& max_number_of_packets
# );
C_MAX_NUMBER_OF_PACKETS = ctypes.c_size_t(10)
CREF_MAX_NUMBER_OF_PACKETS = ctypes.byref(C_MAX_NUMBER_OF_PACKETS)
c_packet_sizes = (ctypes.c_size_t * C_MAX_NUMBER_OF_PACKETS.value)()
c_timestamps = (ctypes.c_int64 * C_MAX_NUMBER_OF_PACKETS.value)()
def comm_p2p_endpoint_recv_packets():
    global wrapper
    packets = []
    if (not wrapper):
        print('Shared library must be loaded in advance!')
        return packets

    wrapper.comm_p2p_endpoint_recv_packets.restype = ctypes.c_size_t

    rx_byte_count = wrapper.comm_p2p_endpoint_recv_packets(
        c_rx_buffer, CREF_RX_BUFFER_SIZE,
        c_packet_sizes, c_timestamps,
        CREF_MAX_NUMBER_OF_PACKETS
    )

    packet_index = 0
    packet_size = 0
    buffer_index = 0

    while ((C_MAX_NUMBER_OF_PACKETS.value > packet_index) and (rx_byte_count > buffer_index)):
        packet_size = c_packet_sizes[packet_index]
        packets.append({
            'data': bytes(c_rx_buffer)[buffer_index:(buffer_index + packet_size)],
            'timestamp_us': c_timestamps[packet_index]
        })

        buffer_index += packet_size
        packet_index += 1

    return packets

####################################################################################################
# Load `libcommw.so` (or `libcommw.dll` on Windows)
####################################################################################################
DEFAULT_CONFIG         = 'config.ini'
CONFIG_MAIN_SECTION    = 'comm-wrapper'
CONFIG_MINGW64_BIN_DIR = 'mingw64_bin_dir'

if ('win32' == sys.platform):
    SHARED_LIB_EXT = '.dll'
    CONFIG_WRAPPER_LIB = 'wrapper_lib_mingw'
else:
    SHARED_LIB_EXT = '.so'
    CONFIG_WRAPPER_LIB = 'wrapper_lib_linux'

def load_library(config_path=None):
    configurations = configparser.ConfigParser()
    if (config_path is None):
        if os.path.isfile(DEFAULT_CONFIG):
            config_path = DEFAULT_CONFIG
        else:
            print('Could not locate configuration file (`{}`)!'.format(DEFAULT_CONFIG))
            return False

    if os.path.isfile(config_path):
        tokens = os.path.splitext(config_path)
        if (tokens is None) or ('.ini' != tokens[-1]):
            print('`{}` is not a valid configuration file!'.format(config_path))
            return False

    configurations.read(config_path)
    sections = configurations.sections()

    if (not sections) or (CONFIG_MAIN_SECTION not in sections):
        print('Missing `{}` section in `{}`'.format())
        return False

    main_section = configurations[CONFIG_MAIN_SECTION]

    if (CONFIG_WRAPPER_LIB not in main_section):
        print('Could not find path to libcommwrapper in {}!'.format(config_path))
        return False

    wrapper_lib_path = main_section[CONFIG_WRAPPER_LIB].strip('"')
    wrapper_lib_path = os.path.abspath(os.path.expanduser(wrapper_lib_path))
    if (not os.path.isfile(wrapper_lib_path)):
        print(wrapper_lib_path, 'does not exists!')
        return False

    wrapper_lib_ext = os.path.splitext(wrapper_lib_path)
    if ((not wrapper_lib_ext) or (wrapper_lib_ext[-1] != SHARED_LIB_EXT)):
        print('`{}` is not a shared library ({})!'.format(wrapper_lib_path, SHARED_LIB_EXT))
        return False

    mingw64_dll_dir = None
    if ('win32' == sys.platform):
        if (CONFIG_MINGW64_BIN_DIR in main_section):
            MINGW64_BIN_DIR = main_section[CONFIG_MINGW64_BIN_DIR].strip('"')
            MINGW64_BIN_DIR = os.path.abspath(os.path.expanduser(MINGW64_BIN_DIR))
            if (os.path.isdir(MINGW64_BIN_DIR)):
                mingw64_dll_dir = os.add_dll_directory(MINGW64_BIN_DIR)
            else:
                print(MINGW64_BIN_DIR, 'does not exist!')
        else:
            print('`{}` is missing in `{}`!'.format(CONFIG_MINGW64_BIN_DIR, config_path))

    global wrapper
    try:
        print('Trying to load `{}` ...'.format(wrapper_lib_path))
        wrapper = ctypes.CDLL(wrapper_lib_path)
    except OSError as e:
        print('-> Could not load `{}`!'.format(wrapper_lib_path))
        print(e)
        return False

    print('-> Loaded successfully!')

    if (mingw64_dll_dir):
        mingw64_dll_dir.close()

    return True
