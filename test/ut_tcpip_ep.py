import comm_wrapper
import time

import test_vectors

TIMEOUT_S = 3

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
        comm_wrapper.comm_p2p_endpoint_send(v)
        print('[{} (us)] Sent packet {} ({} bytes)'.format(time.monotonic_ns()/1000, i, len(v)))

    print('Press enter to check Rx Queue ...')
    input()

    result = True
    i = 0
    t0 = time.monotonic()
    # while (len(test_vectors.vectors) > i):
    #     (rx_buffer, timestamp_us) = comm_wrapper.comm_p2p_endpoint_recv()
    #     if (rx_buffer):
    #         print('Received {} bytes at {} (us)'.format(len(rx_buffer), timestamp_us))
    #         if (ncompare(test_vectors.vectors[i], rx_buffer, len(test_vectors.vectors[i]))):
    #             print('-> Matched!')
    #         else:
    #             print('-> Not matched!')
    #             result = False

    #         i += 1

    #     if (TIMEOUT_S < (time.monotonic() - t0)):
    #         print('Reception timeout!')
    #         result = False
    #         break
    packets = []
    while ((len(packets) < len(test_vectors.vectors)) and (TIMEOUT_S > (time.monotonic() - t0))):
        tmp = comm_wrapper.comm_p2p_endpoint_recv_packets()
        if (tmp):
            packets.extend(tmp)

    print('Received', len(packets), 'packets!')
    while (len(test_vectors.vectors) > i) and (len(packets) > i):
        print('[{} (us)] Packet {} ({} bytes)'.format(
            packets[i]['timestamp_us'], i, len(packets[i]['data']),
        ))
        if (ncompare(test_vectors.vectors[i], packets[i]['data'], len(test_vectors.vectors[i]))):
            print('-> Matched!')
        else:
            print('-> Not matched!')
            result = False

        i += 1

    return result and (len(test_vectors.vectors) == len(packets))
