/* percstr.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "translay.h"

#include "perec.h"

static int HexValue (const unsigned char *from)
{
    int value;

    if (*from>='0' && *from<='9') value = *from - '0';
    else if (*from>='A' && *from<='F') value = *from - 'A' + 10;
    else if (*from>='a' && *from<='f') value = *from - 'a' + 10;
    else {
	fprintf (stderr, "Invalid hex digit '%c'\n", *from);
	exit (33);
    }
    return value;
}

static int HexValue2 (const unsigned char *from)
{
    return (HexValue (from) << 4) + HexValue (from+1);
}

static void UnHex (const unsigned char *from, size_t fromlen,
		   unsigned char *to)
{
    if (fromlen&1) {
	*to++ = HexValue (from);
	++from;
	--fromlen;
    }
    while (fromlen) {
	*to++ = HexValue2 (from);
	from += 2;
	fromlen -= 2;
    }
}

Code *NewString (const BuffData *from, int option)
{
    Code *c;
    void *p;
    size_t len;

    len = from->len;
    if (option==1) len = (from->len+1)/2; /* hex-string */
    else	   len = from->len;

    if (len) {
	p = emalloc (len);
	if (option!=1) memcpy (p, from->ptr, len);

	switch (option) {
	case 1:
	    UnHex ((const void *)from->ptr, from->len, p);
	    break;
#if 'A'==0x41
	case 3: /* EBCDIC */
	    Translay (lat_a2e, len, p);
	    break;
#else
	case 2: /* ASCII */
	    Translay (lat_e2a, len, p);
	    break;
#endif
	default: break;
	}
    } else {
	p = "";
    }
    c = (Code *)ecalloc (1, sizeof (Code));
    c->type = P_CONST;
    c->value.vtype = P_STRING;
    c->value.u.b.ptr = p;
    c->value.u.b.len = len;

    return c;
}
