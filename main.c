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

#include "main.h"

int main(int argc, char ** argv) {
    initOptions();

    for (register int i = 1; i < argc; ++i) {
        parseArgument(argv[i]);
    }

    char strIP[16];
    bool sockOk = false;

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

    FILE * output = NULL;
    if (options.output != NULL) {
        output = fopen(options.output, "w");

        if (output == NULL && options.debug) {
            perror("ERROR (open)");
        }
    }

    for (unsigned int ip = options.ipRange[0]; ip < options.ipRange[1]; ++ip) {
        ipNumToStr(ip, strIP);

        for (unsigned int port = 0; port < options.portsLen; ++port) {
            if (options.debug) {
                printf("Check connection to %s:%hu\n", strIP, options.ports[port]);
            }

            sockOk = checkConnection(ip, options.ports[port]);

            if (sockOk) {
                if (output != NULL) {
                    if (fprintf(output, "%s:%u\n", strIP, options.ports[port]) < 0 && options.debug) {
                        perror("ERROR (fprintf)");
                    }

                    if (fflush(output) == EOF && options.debug) {
                        perror("ERROR (fflush)");
                    }
                }

                printf("IP %s has been responsed on port %hu. (yay!!!)\n", strIP, options.ports[port]);

                break;
            }

            continue;
        }

        if (options.printBoo && !sockOk) {
            printf("IP %s hasn't been responsed. (booooo)\n", strIP);
        }
    }

    if (output != NULL) {
        if (fclose(output) == EOF && options.debug) {
            perror("ERROR (fclose)");
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
