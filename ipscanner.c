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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

typedef char bool;
#define true (1)
#define false (0)

/*
Arguments:
--print-boo     -b      Print bad IP?                             true/false                    true
--debug         -D      Print more info?                          true/false                    false
--ports         -p      Ports for check.                          N unsigned shorts             80 443
--ip-range      -r      First and next to last IP for check.      two unsigned integers         16843009 4294967295
--delay         -d      Connection waiting time.                  seconds, unsigned integer     5
--output        -o      File to save an "ip:port" pairs list.     path to file, string          
*/
struct {
    unsigned short * ports;
    unsigned int * ipRange;

    char * output;

    unsigned int delay;

    unsigned short portsLen;

    bool printBoo;
    bool debug;
} options;

#ifdef __linux__

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define __errno (errno)
#define __error(_desc) { perror("ERROR (" _desc ")"); exit(errno); }

typedef int SOCKET;

const int INVALID_SOCKET = -1;
const int SOCKET_ERROR   = -1;

int __nonBlock(SOCKET sock) {
    static int flags;
    flags = fcntl(sock, F_GETFL, 0);

    return fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

int closesocket(SOCKET sock) {
    return close(sock);
}

#elif _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#define __errno (WSAGetLastError())

const int EINPROGRESS = WSAEWOULDBLOCK;
const int SHUT_RDWR   = SD_BOTH;

void __error(char * desc) {
    static char * msg;
    msg = (char *) malloc(1024);

    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        errno,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR) & msg, 0,
        NULL
    );
    
    fprintf(stderr, "ERROR (%s): %s\n", desc, msg);
    exit(errno);
}

int __nonBlock(SOCKET sock) {
    static unsigned long block;
    block = 1;

    return ioctlsocket(sock, FIONBIO, & block);
}

void sleep(unsigned int seconds) {
    Sleep(seconds * 1000);
}

#endif

void initOptions(void) {
    static unsigned int ipRange[2] = {16843009, 4294967295};
    options.ipRange = ipRange;

    static unsigned short ports[65536] = {80, 443}; 
    options.ports = ports;
    options.portsLen = 2;

    options.delay = 5;

    options.printBoo = false;
    options.debug = false;
    
    options.output = NULL;
}

void resetPorts(void) {
    static unsigned short ports[65536] = {}; 
    options.ports = ports;
    options.portsLen = 0;
}

void printHelpAndExit(void) {
    static const char * help =
    "============{ IP scanner }===========\n\n"
    "Copyright (c) 2018 Eridan Domoratskiy\n"
    "=====================================\n\n"
    "Desription: scans a range of IPv4 addresses by ports\n\n"
    "Usage: "
#ifdef __linux__
    "./ipscanner"
#elif _WIN32
    "ipscanner"
#endif
    " [<options>] [--] [<begin IP>] [<end IP>]\n\n"
    "Begin IP: a first IP to scanning in 255.255.255.255 format\n\n"
    "End IP: a next IP after last to scanning in 255.255.255.255 format\n\n"
    "Options:\n"
    " Full option | Short | Desription                              | Value description            | Default value\n"
    " ------------+-------+-----------------------------------------+------------------------------+--------------\n"
    " --help      |  -h   | Show this message and quit              | *flag*                       | not set\n"
    " --license   |  -l   | Show software license and quit          | *flag*                       | not set\n"
    " ------------+-------+-----------------------------------------+------------------------------+--------------\n"
    " --print-boo |  -b   | Print bad IP?                           | *flag*                       | not set\n"
    " --debug     |  -D   | Print more info?                        | *flag*                       | not set\n"
    " --new-ports |  -P   | Reset ports setted before this flag     | *flag*                       | not set\n"
    " ------------+-------+-----------------------------------------+------------------------------+--------------\n"
    " --ports     |  -p   | Ports for check.                        | N unsigned shorts (0..65535) | 80 443\n"
    " --delay     |  -d   | Connection waiting time.                | seconds, unsigned integer    | 5\n"
    " ------------+-------+-----------------------------------------+------------------------------+--------------\n"
    " --output    |  -o   | File to save an \"ip:port\" pairs list.   | path to file, string         | *nothing*\n"
    "             |       |  !!! FILE WILL BE REWRITTEN ANYWAY !!!  |                              |\n\n";

    printf("%s", help);
    exit(0);
}

void printLicenseAndExit(void) {
    static const char * license =
    "MIT License\n\n"
    "Copyright (c) 2018 Eridan Domoratskiy\n\n"
    "Permission is hereby granted, free of charge, to any person obtaining a copy\n"
    "of this software and associated documentation files (the \"Software\"), to deal\n"
    "in the Software without restriction, including without limitation the rights\n"
    "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
    "copies of the Software, and to permit persons to whom the Software is\n"
    "furnished to do so, subject to the following conditions:\n\n"
    "The above copyright notice and this permission notice shall be included in all\n"
    "copies or substantial portions of the Software.\n\n"
    "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
    "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
    "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
    "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
    "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
    "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
    "SOFTWARE.\n\n";

    printf("%s", license);
    exit(0);
}

void unknownOption(const char * _option, bool shortOption) {
    static const char * option;

    if (shortOption) {
        static char * _opt;
        _opt = (char *) malloc(3);

        sprintf(_opt, "-%c", * _option);

        option = _opt;
    } else {
        option = _option;
    }

    fprintf(stderr, "ERROR: Unknown option \"%s\"\n", option);
    exit(1);
}

int __index(const char * s, char c) {
    static char * ptr;
    ptr = strchr(s, c);

    if (ptr == NULL) {
        return -1;
    }

    return (int) (ptr - s);
}

int strnstr(const char * haystack, const char * needle, size_t len) {
    static size_t haystackLen;
    haystackLen = strlen(haystack);

    for (register int i = 0; i < haystackLen; ++i) {
        if (strncmp(& haystack[i], needle, len) == 0) {
            return i;
        }
    }

    return -1;
}

unsigned int ipStrToNum(const char * ip) {
    static unsigned int ret[4];

    sscanf(ip, "%u.%u.%u.%u",
        & ret[0],
        & ret[1],
        & ret[2],
        & ret[3]
    );

    ret[0] <<= 24;
    ret[1] <<= 16;
    ret[2] <<= 8;

    return ret[0] + ret[1] + ret[2] + ret[3];
}

void parseArgument(const char * arg) {
    static bool ports = false, delay = false, output = false, beginIP = false;

    if (arg[0] == '-') {
        ports = false;
        delay = false;
        output = false;
    }

    if (
        arg[0] == '-' &&
        arg[1] != '-'
    ) {
        static const char * availableArgs = "bdDhoplP";
        ++arg;

        for (; (* arg) != '\0'; ++arg) {
            static int option;
            option = __index(availableArgs, * arg);

            switch (option) {
            case 0:
                options.printBoo = true;
                break;
            case 1:
                delay = true;
                break;
            case 2:
                options.debug = true;
                break;
            case 3:
                printHelpAndExit();
                break;
            case 4:
                output = true;
                break;
            case 5:
                ports = true;
                break;
            case 6:
                printLicenseAndExit();
                break;
            case 7:
                resetPorts();
                break;
            default:
                unknownOption(& availableArgs[option], true);
                break;
            }
        }

        return;
    }

    if (arg[0] == '-') {
        if (arg[3] == '\0') {
            return;
        }

        static const char * availableArgs = "prin" "dela" "debu" "help" "outp" "port" "lice" "new-";
        arg += 2;

        static enum {
            UNKNOWN   = -1,
            PRINT_BOO = 0,
            DELAY     = 4,
            DEBUG     = 8,
            HELP      = 12,
            OUTPUT    = 16,
            PORTS     = 20,
            LICENSE   = 24,
            NEW_PORTS = 28
        } option;
        option = strnstr(availableArgs, arg, 4);

        switch (option) {
        case PRINT_BOO:
            options.printBoo = true;
            break;
        case DELAY:
            delay = true;
            break;
        case DEBUG:
            options.debug = true;
            break;
        case HELP:
            printHelpAndExit();
            break;
        case OUTPUT:
            output = true;
            break;
        case PORTS:
            ports = true;
            break;
        case LICENSE:
            printLicenseAndExit();
            break;
        case NEW_PORTS:
            resetPorts();
            break;
        default:
            unknownOption(arg - 2, false);
            break;
        }

        {
            static int i;
            i = __index(arg, '=');

            if (i != -1) {
                arg += i + 1;
            } else {
                return;
            }
        }
    }

    if (ports) {
        sscanf(arg, "%hu", & options.ports[options.portsLen++]);
        return;
    }

    if (delay) {
        sscanf(arg, "%u", & options.delay);

        delay = false;
        return;
    }

    if (output) {
        options.output = malloc(strlen(arg) + 1);
        strcpy(options.output, arg);

        output = false;
        return;
    }

    if (!beginIP) {
        options.ipRange[0] = ipStrToNum(arg);

        beginIP = true;
        return;
    }

    options.ipRange[1] = ipStrToNum(arg);
}

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

bool checkConnection(unsigned int ip, unsigned int port) {
    static bool sockOk;

    static SOCKET sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        __error("socket");
    }

    if (__nonBlock(sock) == SOCKET_ERROR) {
        __error("__nonBlock");
    }

    static struct sockaddr_in sockAddr;
    memset(& sockAddr, 0, sizeof(sockAddr));

    sockAddr.sin_family = PF_INET;
    sockAddr.sin_port = htons(port);
    ipNumToAddr(ip, & sockAddr.sin_addr);

    if (
        connect(sock, (struct sockaddr *) & sockAddr, sizeof(sockAddr)) == SOCKET_ERROR &&
        __errno == EINPROGRESS
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
                getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *) & error, & errLen) == SOCKET_ERROR ||
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

    if (sockOk && shutdown(sock, SHUT_RDWR) == SOCKET_ERROR) {
        __error("shutdown");
    }

    if (closesocket(sock) == SOCKET_ERROR) {
        __error("closesocket");
    }

    return sockOk;
}

int main(int argc, char ** argv) {
    initOptions();

    for (register int i = 1; i < argc; ++i) {
        parseArgument(argv[i]);
    }

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

    int output;
    if (options.output != NULL) {
        output = open(options.output, O_WRONLY);

        if (output == -1 && options.debug) {
            perror("ERROR (open)");
        }
    }

    for (unsigned int ip = options.ipRange[0]; ip < options.ipRange[1]; ++ip) {
        ipNumToStr(ip, strIP);

        for (unsigned int port = 0; port < options.portsLen; ++port) {
            if (options.debug) {
                printf("Check connection to %s:%d\n", strIP, options.ports[port]);
            }

            sockOk = checkConnection(ip, options.ports[port]);

            if (sockOk) {
                if (output != -1) {
                    if (dprintf(output, "%s:%u\n", strIP, options.ports[port]) < 0 && options.debug) {
                        perror("ERROR (dprintf)");
                    }
                }

                printf("IP %s has been responsed on port %u. (yay!!!)\n", strIP, options.ports[port]);

                break;
            }

            continue;
        }

        if (options.printBoo && !sockOk) {
            printf("IP %s hasn't been responsed. (booooo)\n", strIP);
        }
    }

    if (output != -1) {
        if (close(output) == -1 && options.debug) {
            perror("ERROR (close)");
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
