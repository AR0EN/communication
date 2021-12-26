import sys

sys.path.append('../wrapper')
import comm_wrapper
import time

import test_vectors

TIMEOUT_S = 3
SINGLE_PACKET = False

def ncompare(buf0, buf1, n):
    if (0 >= n):
        print('3rd argument, size of interest,  must be a positive integer!')
        return False

    if (n > len(buf0)):
        print('Length of 1st buffer ({}) is less than expected ({})!'.format(len(buf0, n)))
        return False

    if (n > len(buf0)):
        print('Length of 2nd buffer ({}) is less than expected ({})!'.format(len(buf1, n)))
        return False

    for i in range(n):
        if (buf0[i] != buf1[i]):
            return False

    return True

def execute():
    if (not comm_wrapper.comm_endpoint_ready()):
        print('Endpoint is not ready!')
        return False

    print('Endpoint is ready, waiting for peer ...')

    t0 = time.monotonic()
    while ((TIMEOUT_S > (time.monotonic() - t0)) and (not comm_wrapper.comm_peer_connected())):
        pass

    if (not comm_wrapper.comm_peer_connected()):
        print('Connection timeout!')
        return False

    print('Connected to peer, press enter to sent data to peer ...')
    input()

    for i, v in enumerate(test_vectors.vectors):
        if (comm_wrapper.comm_p2p_endpoint_send(v)):
            print('[{} (us)] Sent packet {} ({} bytes)!'.format(time.monotonic_ns()/1000, i, len(v)))
        else:
            print('[{} (us)] Failed to send packet {} ({} bytes)!'.format(time.monotonic_ns()/1000, i, len(v)))

    print('Press enter to check Rx Queue ...')
    input()
   
    packets = []
    EXPECTED_NUMBER_OF_PACKETS = len(test_vectors.vectors)
    t0 = time.monotonic()
    while ((len(packets) < EXPECTED_NUMBER_OF_PACKETS) and (TIMEOUT_S > (time.monotonic() - t0))):
        if (SINGLE_PACKET):
            packet = comm_wrapper.comm_p2p_endpoint_recv_packet()
            if (packet):
                packets.append(packet)
        else:
            tmp = comm_wrapper.comm_p2p_endpoint_recv_packets()
            if (tmp):
                packets.extend(tmp)   

    NUMBER_OF_RX_PACKETS = len(packets)
    print('Received', NUMBER_OF_RX_PACKETS, 'packets!')

    i = 0
    result = (EXPECTED_NUMBER_OF_PACKETS == NUMBER_OF_RX_PACKETS)
    while (EXPECTED_NUMBER_OF_PACKETS > i) and (NUMBER_OF_RX_PACKETS > i):
        print('[{} (us)] Packet {} ({} bytes)'.format(
            packets[i][comm_wrapper.PACKET_KEY_TIMESTAMP], i, len(packets[i][comm_wrapper.PACKET_KEY_DATA]),
        ))
        if (
            ncompare(test_vectors.vectors[i], packets[i][comm_wrapper.PACKET_KEY_DATA], 
                len(test_vectors.vectors[i])
            )
        ):
            print('  -> Matched!')
        else:
            print('  -> Not matched!')
            result = False

        i += 1

    return result
