#ifdef __linux__

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#elif _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

typedef short bool;
#define true (1);
#define false (0);

/* => CONFIGURATION <=

printBoo     - Print bad IP?                        (true/false)
debug        - Print more info?                     (true/false)
ports        - Ports for check.                     (unsigned integers)
portsLen     - Count of ports.                      (unsigned integers)
ipRange      - First and next to last IP for check. (unsigned integer)
connDelay    - Connection waiting time.             (seconds, unsigned integer)

*/

bool printBoo = true;
bool debug = true;

const unsigned int ports[] = {
    80,  // HTTP
    443  // HTTPS
    //, 13, 11, 20, 21, 22, 23, 25, 43, 81, 82, 101, 109, 110, 130, 131, 132, 143, 194, 689
};
const unsigned int portsLen = 2;

const unsigned int ipRange[] = {
    1370590694, //  81.177.141.230   // 16843009,   //   1.  1.  1.  1
    4294967295  // 255.255.255.255
};

unsigned int connDelay = 5;

/* => CONFIGURATION END <= */

void ipNumToAddr(unsigned int ip, struct in_addr * dst) {
    char addr[4];

    addr[0] = (ip >> 24) & 0xff;
    addr[1] = (ip >> 16) & 0xff;
    addr[2] = (ip >> 8) & 0xff;
    addr[3] = ip & 0xff;

    memcpy(dst, addr, 4);
}

void ipNumToStr(unsigned int ip, char * dst) {
    sprintf(dst, "%u.%u.%u.%u",
        (ip >> 24) & 0xff,
        (ip >> 16) & 0xff,
        (ip >> 8) & 0xff,
        ip & 0xff
    );
}

#ifdef __linux__

bool checkConnection(unsigned int ip, unsigned int port) {
    static bool sockOk;

    static int sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("ERROR (socket)");
        exit(errno);
    }

    static flags;
    flags = fcntl(sock, F_GETFL, 0);
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1 && debug) {
        perror("ERROR (fcntl O_NONBLOCK)");
        exit(errno);
    }

    static struct sockaddr_in sockAddr;
    memset(& sockAddr, 0, sizeof(sockAddr));

    sockAddr.sin_family = PF_INET;
    sockAddr.sin_port = htons(port);
    ipNumToAddr(ip, & sockAddr.sin_addr);

    if (
        connect(sock, (struct sockaddr *) & sockAddr, sizeof(sockAddr)) == -1 &&
        errno == EINPROGRESS
    ) {
        static int maxFd;
        static fd_set rfds, wfds;
        static struct timeval tv;

        tv.tv_sec = 0;
        tv.tv_usec = 500;
        FD_ZERO(& rfds); 
        FD_ZERO(& wfds);

        for (unsigned int i = 0; i < connDelay; ++i) {
            sleep(1);

            FD_SET(sock, & wfds);
            FD_SET(sock, & rfds);
            if(sock > maxFd) {
                maxFd = sock;
            }

            select(maxFd + 1, & rfds, & wfds, NULL, & tv);

            if (FD_ISSET(sock, & wfds) || FD_ISSET(sock, & rfds)) {
                break;
            }
        }

        if (!FD_ISSET(sock, & wfds) && !FD_ISSET(sock, & rfds)) {
            sockOk = false;

            if (debug) {
                fprintf(stderr, "ERROR (connect): Timed out\n");
            }
        } else {
            static socklen_t errLen;
            static int error;

            errLen = sizeof(error);
            if (
                getsockopt(sock, SOL_SOCKET, SO_ERROR, & error, & errLen) == -1 ||
                error != 0
            ) {
                sockOk = false;

                if (debug) {
                    fprintf(stderr, "ERROR (connect): Socket error\n");
                }
            } else {
                sockOk = true;
            }
        }
    }

    if (sockOk && shutdown(sock, SHUT_RDWR) == -1) {
        perror("ERROR (shutdown)");
        exit(errno);
    }

    if (close(sock) == -1) {
        perror("ERROR (close)");
        exit(errno);
    }

    return sockOk;
}

#elif _WIN32

bool checkConnection(unsigned int ip, unsigned int port) {
    static bool sockOk;

    static SOCKET sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "ERROR (socket)\n");
        exit(WSAGetLastError());
    }

    static unsigned long block = 1;
    ioctlsocket(sock, FIONBIO, & block);

    static struct sockaddr_in sockAddr;
    memset(& sockAddr, 0, sizeof(sockAddr));

    sockAddr.sin_family = PF_INET;
    sockAddr.sin_port = htons(port);
    ipNumToAddr(ip, & sockAddr.sin_addr);

    if (
        connect(sock, (struct sockaddr *) & sockAddr, sizeof(sockAddr)) == SOCKET_ERROR &&
        WSAGetLastError() == WSAEWOULDBLOCK
    ) {
        static fd_set rfds, wfds;
        static struct timeval tv;

        tv.tv_sec = 0;
        tv.tv_usec = 500;
        FD_ZERO(& rfds); 
        FD_ZERO(& wfds);

        for (unsigned int i = 0; i < connDelay; ++i) {
            Sleep(1000);

            FD_SET(sock, & wfds);
            FD_SET(sock, & rfds);

            select(0, & rfds, & wfds, NULL, & tv);

            if (FD_ISSET(sock, & wfds) || FD_ISSET(sock, & rfds)) {
                break;
            }
        }

        if (!FD_ISSET(sock, & wfds) && !FD_ISSET(sock, & rfds)) {
            sockOk = false;

            if (debug) {
                fprintf(stderr, "ERROR (connect): Timed out\n");
            }
        } else {
            static socklen_t errLen;
            static int error;

            errLen = sizeof(error);
            if (
                getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *) & error, & errLen) == SOCKET_ERROR ||
                error != 0
            ) {
                sockOk = false;

                if (debug) {
                    fprintf(stderr, "ERROR (connect): Socket error\n");
                }
            } else {
                sockOk = true;
            }
        }
    }

    if (sockOk && shutdown(sock, SD_BOTH) == SOCKET_ERROR) {
        fprintf(stderr, "ERROR (shutdown)\n");
        exit(WSAGetLastError());
    }

    if (closesocket(sock) == SOCKET_ERROR) {
        fprintf(stderr, "ERROR (closesocket)\n");
        exit(WSAGetLastError());
    }

    return sockOk;
}

#endif

int main(int argc, char ** argv) {
    char strIP[16];
    bool sockOk;

#ifdef _WIN32

    WSADATA lpWSAData;
    {
        int result = WSAStartup(MAKEWORD(1, 1), & lpWSAData);
        if (result != 0) {
            fprintf(stderr, "ERROR (WSAStartup)\n");
            exit(result);
        }
    }

#endif

    for (unsigned int ip = ipRange[0]; ip < ipRange[1]; ++ip) {
        ipNumToStr(ip, strIP);

        for (unsigned int port = 0; port < portsLen; ++port) {
            if (debug) {
                printf("Check connection to %s:%d\n", strIP, ports[port]);
            }

            sockOk = checkConnection(ip, ports[port]);

            if (sockOk) {
                printf("IP %s has been responsed on port %u. (yay!!!)\n", strIP, ports[port]);
                break;
            } else {
                continue;
            }
        }

        if (printBoo && !sockOk) {
            printf("IP %s hasn't been responsed. (booooo)\n", strIP);
        }
    }

#ifdef _WIN32

    if (WSACleanup() == SOCKET_ERROR) {
        fprintf(stderr, "ERROR (WSACleanup)\n");
        exit(WSAGetLastError());
    }

#endif

    return 0;
}