/* MIT License

Copyright (c) 2018 Eridan Domoratskiy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#ifdef __linux__

#include "linux.h"

#define __error(_desc) { perror("ERROR (" _desc ")"); exit(errno); }

int setSocketNonBlock(int sock) {
    static int flags;
    flags = fcntl(sock, F_GETFL, 0);

    return fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

bool checkConnection(unsigned int ip, unsigned int port) {
    static bool sockOk;

    static int sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        __error("socket");
    }

    if (setSocketNonBlock(sock) == -1) {
        __error("setSocketNonBlock");
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
        static int maxFd = 0;
        static fd_set rfds, wfds;
        static struct timeval tv;

        tv.tv_sec = 0;
        tv.tv_usec = 500;
        FD_ZERO(& rfds);
        FD_ZERO(& wfds);

        for (register unsigned int i = 0; i < options.delay; ++i) {
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

            if (options.debug) {
                fprintf(stderr, "ERROR (connect): Timed out\n");
            }
        } else {
            static socklen_t errLen;
            static int error;

            errLen = sizeof(error);
            if (
                getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *) & error, & errLen) == -1 ||
                error != 0
            ) {
                sockOk = false;

                if (options.debug) {
                    fprintf(stderr, "ERROR (connect): Socket error\n");
                }
            } else {
                sockOk = true;
            }
        }
    }

    if (sockOk && shutdown(sock, SHUT_RDWR) == -1) {
        __error("shutdown");
    }

    if (close(sock) == -1) {
        __error("close");
    }

    return sockOk;
}

#endif
