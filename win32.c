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

#ifdef _WIN32

#include "win32.h"

void error(char * desc) {
    static char * msg;
    msg = (char *) malloc(1024);

    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR) & msg, 0,
        NULL
    );

    fprintf(stderr, "ERROR (%s): %s\n", desc, msg);
    exit(WSAGetLastError());
}

int setSocketNonBlock(SOCKET sock) {
    static unsigned long block;
    block = 1;

    return ioctlsocket(sock, FIONBIO, & block);
}

bool checkConnection(unsigned int ip, unsigned int port) {
    static bool sockOk;

    static SOCKET sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        error("socket");
    }

    if (setSocketNonBlock(sock) == SOCKET_ERROR) {
        error("setSocketNonBlock");
    }

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

        for (register unsigned int i = 0; i < options.delay; ++i) {
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

    if (sockOk && shutdown(sock, SD_BOTH) == SOCKET_ERROR) {
        error("shutdown");
    }

    if (closesocket(sock) == SOCKET_ERROR) {
        error("closesocket");
    }

    return sockOk;
}

#endif
