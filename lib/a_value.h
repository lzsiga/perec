/* a_value.h */
 
#ifndef A_VALUE_H
#define A_VALUE_H

#include <stddef.h>

typedef struct ValuePar {
    int     BASE;          /* BASE (I), THE NUMBER (0) */
    size_t  LEN;           /* LEN (I/O) */
    void   *ADDR;          /* ADDR (I/O) */
} ValuePar;
 
int A_Value (ValuePar *);

int B_Value   (size_t len, const char *p, int *ret);
int B_ValueL  (size_t len, const char *p, long *ret);
int B_ValueLL (size_t len, const char *p, long long *ret);

/* 0=ok, 1=üres, -1=hiba */

#endif
