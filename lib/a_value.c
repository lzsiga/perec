/* a_value.c */

#include <ctype.h>
#include <stddef.h>

#include "a_value.h"

#if !defined(BS2000) && !defined(SIE_BS2000) 
int A_Value (ValuePar *par)
{
    char *p, *q;
    int sign= 1;
    int rc= -1;
    int num= 0;
    int c;

    p = par->ADDR;
    q = p + par->LEN;
    while (p<q && *p==' ') ++p;

    while (p<q && q[-1]==' ') --q;

    if (p<q && *p=='-') {
	sign= -1;
	++p;
    }

    if (p==q) goto RET;
    while (p<q) {
	if (isdigit (*p)) c = *p - '0';
	else if (islower (*p)) c = *p - 'a' + 10;
	else if (isupper (*p)) c = *p - 'A' + 10;
	else goto RET;
	++p;

	if (c>par->BASE) goto RET;
	num = num*par->BASE + c;
    }
    rc = 0;
    par->BASE = sign*num;

RET:
    par->LEN -= p - (char *)par->ADDR;
    par->ADDR = p;
    return rc;
}
#endif

int B_Value (size_t len, const char *p, int *ret)
{
    int rc;
    long long lval;

    rc = B_ValueLL (len, p, &lval);
    *ret= (int)lval;
    return rc;
}

int B_ValueL (size_t len, const char *p, long *ret)
{
    int rc;
    long long lval;

    rc = B_ValueLL (len, p, &lval);
    *ret= (long)lval;
    return rc;
}

int B_ValueLL (size_t len, const char *p, long long *ret)
{
    const char *q;
    int sign= 1;
    int rc= -1;
    long long num= 0;
    int c;

    q = p + len;

    while (p<q && *p==' ') ++p;
    while (p<q && q[-1]==' ') --q;

    if (p>=q) { /* üres */
	rc= 1;
	goto RET;
    }

    if (*p=='-') {
	sign= -1;
	if (++p==q) goto RET;
    }

    while (p<q) {
	c = *(unsigned char *)p;
	if (! isdigit (c)) goto RET;
	num = num*10 + (c-'0');
	++p;
    }
    rc = 0;

RET:
    *ret = sign*num;
    return rc;
}
