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

#include "options.h"

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
    "\t--help (-h)\n"
    "\t  Show this message and quit.\n"
    "\t--license (-l)\n"
    "\t  Show software license and quit.\n"
    "\t--print-boo (-b)\n"
    "\t  Print bad IP?\n"
    "\t--debug (-D)\n"
    "\t  Print more info?\n"
    "\t--new-ports (-P)\n"
    "\t  Reset ports setted before this flag.\n"
    "\t--ports (-p)\n"
    "\t  Ports for check, N numbers in from 0 to 65535. Default: 80 443.\n"
    "\t--delay (-d)\n"
    "\t  Connection waiting time, seconds. Default: 5 sec.\n"
    "\t--output (-o)\n"
    "\t  File to save an \"ip:port\" pairs list, path to file. Default: not setted.\n"
    "\t  !!! FILE WILL BE REWRITTEN ANYWAY !!!\n\n";

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

        snprintf(_opt, 2, "-%c", * _option);

        option = _opt;
    } else {
        option = _option;
    }

    fprintf(stderr, "ERROR: Unknown option \"%s\"\n", option);
    exit(1);
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
            option = strchri(availableArgs, * arg);

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

        static const char * availableArgs = "print-boo" "delay" "debug" "help" "output" "ports" "license" "new-ports";
        arg += 2;

        static enum {
            UNKNOWN   = -1,
            PRINT_BOO = 0,
            DELAY     = 9,
            DEBUG     = 14,
            HELP      = 19,
            OUTPUT    = 23,
            PORTS     = 29,
            LICENSE   = 34,
            NEW_PORTS = 28
        } option;
        option = strstri(availableArgs, arg);

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
            i = strchri(arg, '=');

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
        static size_t s;
        s = strlen(arg) + 1;

        options.output = malloc(s);
        strncpy(options.output, arg, s);

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
