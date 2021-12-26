import sys

sys.path.append('../wrapper')
import comm_wrapper
import ut_tcpip_ep

if __name__ == "__main__":
    if (3 > len(sys.argv)):
        print('Usage:', sys.argv[0], '<Server Address> <Server Port>\n')
        exit

    if (not comm_wrapper.load_library('config.ini')):
        print('Could not load shared library!')
        exit()

    print('Initializing TCP Client ...')
    if (comm_wrapper.comm_tcp_client_init(sys.argv[1], int(sys.argv[2]))):
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
