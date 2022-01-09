#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <unistd.h>

#include <chrono>
#include <thread>

#define LOGI(...)   printf(__VA_ARGS__)
#define LOGE(...)   fprintf(stderr, __VA_ARGS__)

typedef int SOCKET;

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 1234

#define RX_TIMEOUT_S 10

const char MSG[] = "Thank you!";
uint8_t rx_buffer[128];

inline std::chrono::time_point<std::chrono::steady_clock> get_monotonic_clock() {
    return std::chrono::steady_clock::now();
}

int main(int argc, char ** argv) {
    int ret;

    SOCKET socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > socketFd) {
        perror("Could not create TCP socket!\n");
        return -1;
    }

    int enable = 1;
    ret = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    if (0 > ret) {
        perror("Failed to enable SO_REUSEADDR!\n");
        ::close(socketFd);
        return -1;
    }

    int flags = fcntl(socketFd, F_GETFL, 0);
    if (0 > flags) {
        perror("Failed to get socket flags!\n");
        ::close(socketFd);
        return -1;
    }

    if (0 > fcntl(socketFd, F_SETFL, (flags | O_NONBLOCK))) {
        perror("Failed to enable NON-BLOCKING mode!\n");
        ::close(socketFd);
        return -1;
    }

    struct sockaddr_in remoteSocketAddr;
    remoteSocketAddr.sin_family      = AF_INET;
    remoteSocketAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    remoteSocketAddr.sin_port        = htons(SERVER_PORT);

    auto t0 = get_monotonic_clock();

    do {
        ret = connect(socketFd, reinterpret_cast<const struct sockaddr *>(&remoteSocketAddr), sizeof(remoteSocketAddr));
        if (0 == ret) {
            break;
        }

    } while (
        RX_TIMEOUT_S > std::chrono::duration_cast<std::chrono::seconds>(
                            get_monotonic_clock() - t0
                        ).count()
    );

    if (0 != ret) {
        perror("Failed to connect to server!\n");
        ::close(socketFd);
        return -1;
    }

    LOGI("[%s][%d] Connected to %s/%u, waiting for messages from server ...\n", __func__, __LINE__, SERVER_ADDR, SERVER_PORT);

    memset(rx_buffer, 0, sizeof(rx_buffer));
    while (true) {
        ret = ::recv(socketFd, rx_buffer, sizeof(rx_buffer), 0);
        if (0 > ret) {
            if (EWOULDBLOCK == errno) {
                continue;
            } else {
                perror("Failed to read from TCP Socket!");
                ::close(socketFd);
                return -1;
            }
        } else if (0 == ret) {
            LOGE("[%s][%d] Stream socket peer has performed an orderly shutdown!\n", __func__, __LINE__);
            ::close(socketFd);
            return -1;
        } else {
            LOGI("[%s][%d] Received %d bytes: '%s' after waiting for %lld (ms)\n",
                __func__, __LINE__,
                ret, rx_buffer,
                static_cast<long long int>(std::chrono::duration_cast<std::chrono::milliseconds>(get_monotonic_clock() - t0).count())
            );
            break;
        }
    }

    while (true) {
        ret = ::send(socketFd, MSG, sizeof(MSG), 0);
        if (0 > ret) {
            if (EWOULDBLOCK == errno) {
                continue;
            } else {
                perror("Failed to write to TCP Socket!");
                ::close(socketFd);
                return -1;
            }
        } else if (0 == ret) {
            LOGE("[%s][%d] Should not happen!\n", __func__, __LINE__);
            ::close(socketFd);
            return -1;
        } else {
            LOGI("[%s][%d] Transmitted %d bytes\n", __func__, __LINE__, ret);
            break;
        }
    }

    memset(rx_buffer, 0, sizeof(rx_buffer));
    t0 = get_monotonic_clock();
    while (true) {
        ret = ::recv(socketFd, rx_buffer, sizeof(rx_buffer), 0);
        if (0 > ret) {
            if (EWOULDBLOCK == errno) {
                continue;
            } else {
                perror("Failed to read from TCP Socket!");
                ::close(socketFd);
                return -1;
            }
        } else if (0 == ret) {
            LOGE("[%s][%d] Stream socket peer has performed an orderly shutdown!\n", __func__, __LINE__);
            ::close(socketFd);
            return -1;
        } else {
            LOGI("[%s][%d] Received %d bytes: '%s' after waiting for %lld (ms)\n",
                __func__, __LINE__,
                ret, rx_buffer,
                static_cast<long long int>(std::chrono::duration_cast<std::chrono::milliseconds>(get_monotonic_clock() - t0).count())
            );
            break;
        }
    }

    LOGE("[%s][%d] Terminating ...\n", __func__, __LINE__);

    ::close(socketFd);
    return 0;
}
