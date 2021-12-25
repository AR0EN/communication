import configparser
import ctypes
import os
import sys

# bool comm_tcp_client_init(const char * const server_addr, const uint16_t& server_port);
# bool comm_tcp_server_init(const uint16_t& port);
# bool comm_udp_peer_init(const uint16_t& local_port, const char * const remote_addr, const uint16_t& remote_port);

# void comm_deinit();

# bool comm_p2p_endpoint_send(const uint8_t * const buffer, const size_t& buffer_size);
# ssize_t comm_p2p_endpoint_recv(uint8_t * const buffer, const size_t& buffer_size);

DEFAULT_CONFIG         = 'config.ini'
CONFIG_MAIN_SECTION    = 'comm-wrapper'
CONFIG_MINGW64_BIN_DIR = 'mingw64_bin_dir'
CONFIG_WRAPPER_LIB     = 'wrapper_lib_path'
CONFIG_LIB_EXTS        = 'shared_lib_exts'

wrapper = None

def load_wrapper(config_path=None):
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

    if ('win32' == sys.platform):
        MINGW64_BIN_DIR =
        os.add_dll_directory()
