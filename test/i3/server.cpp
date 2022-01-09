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

#define SERVER_PORT 1234
#define BACKLOG 5

const char MSG0[] = "Hello!";
const char MSG1[] = "Goodbye!";
uint8_t rx_buffer[128];

inline std::chrono::time_point<std::chrono::steady_clock> get_monotonic_clock() {
    return std::chrono::steady_clock::now();
}

int main(int argc, char ** argv) {
    int ret;

    SOCKET localSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > localSocketFd) {
        perror("Could not create TCP socket!\n");
        return -1;
    }

    int enable = 1;
    ret = setsockopt(localSocketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    if (0 > ret) {
        perror("Failed to enable SO_REUSEADDR!\n");
        ::close(localSocketFd);
        return -1;
    }

    struct sockaddr_in localSocketAddr;
    localSocketAddr.sin_family      = AF_INET;
    localSocketAddr.sin_addr.s_addr = INADDR_ANY;
    localSocketAddr.sin_port = htons(SERVER_PORT);

    ret = bind(localSocketFd, reinterpret_cast<const struct sockaddr *>(&localSocketAddr), sizeof(localSocketAddr));
    if (0 > ret) {
        perror("Failed to assigns address to the socket!\n");
        ::close(localSocketFd);
        return -1;
    }

    ret = listen(localSocketFd, BACKLOG);

    if (0 != ret) {
        perror("Failed to mark the socket as a passive socket!\n");
        ::close(localSocketFd);
        return -1;
    }

    LOGI("[%s][%d] TCP Server is listenning at port %u ...\n", __func__, __LINE__, SERVER_PORT);

    struct sockaddr_in remoteSocketAddr;
    socklen_t remoteAddressSize = static_cast<socklen_t>(sizeof(remoteSocketAddr));

    SOCKET mRemotePipeFd;

    while(true) {
        mRemotePipeFd = accept(
                            localSocketFd,
                            reinterpret_cast<struct sockaddr*>(&remoteSocketAddr),
                            &remoteAddressSize
                    );

        if (0 >= mRemotePipeFd) {
            if (EAGAIN == errno) {
                // Timeout - do nothing
                continue;
            } else {
                perror("Could not access connection queue!\n");
                ::close(localSocketFd);
                return -1;
            }
        }

        int flags = fcntl(mRemotePipeFd, F_GETFL, 0);
        if (0 > flags) {
            perror("Failed to get socket flags!\n");
            ::close(mRemotePipeFd);
            continue;
        }

        if (0 > fcntl(mRemotePipeFd, F_SETFL, (flags | O_NONBLOCK))) {
            perror("Failed to enable NON-BLOCKING mode!\n");
            ::close(mRemotePipeFd);
            continue;
        }

        while (true) {
            ret = ::send(mRemotePipeFd, MSG0, sizeof(MSG0), 0);
            if (0 > ret) {
                if (EWOULDBLOCK == errno) {
                    continue;
                } else {
                    perror("Failed to write to TCP Socket!");
                }
            } else if (0 == ret) {
                LOGE("[%s][%d] Should not happen!\n", __func__, __LINE__);
            } else {
                LOGI("[%s][%d] Transmitted %d bytes\n", __func__, __LINE__, ret);
            }

            break;
        }

        memset(rx_buffer, 0, sizeof(rx_buffer));
        auto t0 = get_monotonic_clock();
        while (true) {
            ret = ::recv(mRemotePipeFd, rx_buffer, sizeof(rx_buffer), 0);
            if (0 > ret) {
                if (EWOULDBLOCK == errno) {
                    continue;
                } else {
                    perror("Failed to read from TCP Socket!");
                }
            } else if (0 == ret) {
                LOGE("[%s][%d] Stream socket peer has performed an orderly shutdown!\n", __func__, __LINE__);
            } else {
                LOGI("[%s][%d] Received %d bytes: '%s' after waiting for %lld (ms)\n",
                    __func__, __LINE__,
                    ret, rx_buffer,
                    static_cast<long long int>(std::chrono::duration_cast<std::chrono::milliseconds>(get_monotonic_clock() - t0).count())
                );
            }

            break;
        }

        while (true) {
            ret = ::send(mRemotePipeFd, MSG1, sizeof(MSG1), 0);
            if (0 > ret) {
                if (EWOULDBLOCK == errno) {
                    continue;
                } else {
                    perror("Failed to write to TCP Socket!");
                }
            } else if (0 == ret) {
                LOGE("[%s][%d] Should not happen!\n", __func__, __LINE__);
            } else {
                LOGI("[%s][%d] Transmitted %d bytes\n", __func__, __LINE__, ret);
            }

            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        ::close(mRemotePipeFd);
    }

    LOGE("[%s][%d] Terminating ...\n", __func__, __LINE__);

    ::close(localSocketFd);
    return 0;
}
