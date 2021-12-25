import sys

import comm_wrapper
import ut_tcpip_ep

if __name__ == "__main__":
    if (4 > len(sys.argv)):
        print('Usage:', sys.argv[0], '<Local Port> <Peer Address> <Peer Port>')
        exit

    if (not comm_wrapper.load_library('config.ini')):
        print('Could not load shared library!')
        exit()

    print('Initializing Udp Endpoint ...')
    if (comm_wrapper.comm_udp_peer_init(int(sys.argv[1]), sys.argv[2], int(sys.argv[3]))):
        print('-> Successfully!')
    else:
        print('-> Failed!')

    if (ut_tcpip_ep.execute()):
        print('-> Passed!')
    else:
        print('-> Failed!')

    comm_wrapper.comm_deinit()

    print('Press enter to exit ...')
    input()
