/* translay.c */

#include <string.h>

#include "translay.h"

void Translay (const unsigned char map[256], size_t n, void *buff)
{
    unsigned char *p;

    p = buff;
    while (n) {
        *p = map[*p];
        ++p;
        --n;
    }
}

void TransMov (const unsigned char map[256], void *to,
               size_t len, const void *from)
{
    const unsigned char *p;
    unsigned char *q;

    p = from;
    q = to;
    while (len) {
        *q++ = map[*p++];
        --len;
    }
}

void TransMovX (const unsigned char map[256],
                size_t tolen,   void *to,
                size_t fromlen, const void *from,
                char fill)
{
    const unsigned char *p;
    unsigned char *q;
    size_t cpylen;

    cpylen= fromlen;
    if (cpylen>tolen) cpylen= tolen;

    p = from;
    q = to;
    while (cpylen) {
        *q++ = map[*p++];
        --cpylen;
    }

    if (tolen>fromlen) {
        memset (q, fill, tolen-fromlen);
    }
}
