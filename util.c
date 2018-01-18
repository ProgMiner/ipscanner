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

#include "util.h"

int strchri(const char * haystack, char needle) {
    static char * ptr;
    ptr = strchr(haystack, needle);

    if (ptr == NULL) {
        return -1;
    }

    return (int) (ptr - haystack);
}

int strstri(const char * haystack, const char * needle) {
    static char * ptr;
    ptr = strstr(haystack, needle);

    if (ptr == NULL) {
        return -1;
    }

    return (int) (ptr - haystack);
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

unsigned int ipStrToNum(const char * ip) {
    static unsigned int ret[4];

    sscanf(ip, "%3u.%3u.%3u.%3u",
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
