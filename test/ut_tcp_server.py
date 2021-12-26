import sys

sys.path.append('../wrapper')
import comm_wrapper
import ut_tcpip_ep

if __name__ == "__main__":
    if (2 > len(sys.argv)):
        print('Usage:', sys.argv[0], ' <Local Port>')
        exit

    if (not comm_wrapper.load_library('config.ini')):
        print('Could not load shared library!')
        exit()

    print('Initializing TCP Server ...')
    if (comm_wrapper.comm_tcp_server_init(int(sys.argv[1]))):
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
