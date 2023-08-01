/* c_mvcl.c */

#include <stdlib.h>
#include <string.h>

#include "a_mvcl.h"
#include "defs.h"
#include "buffdata.h"

extern size_t strnlen (const char *from, size_t max);

#if !defined(BS2000) && !defined(SIE_BS2000)

void *a_mvcl (size_t tolen, void *to,
	      size_t fromlen, const void *from, char fill)
{
    size_t cpylen;

    if (tolen<fromlen) cpylen= tolen;
    else	       cpylen= fromlen;
    memcpy (to, from, cpylen);
    if (tolen>fromlen)
	memset ((char *)to + fromlen, fill, tolen-fromlen);
    return to;
}

void *a_mvcr (size_t tolen, void *to,
	      size_t fromlen, const void *from, char fill)
{
    void *cpyto;
    const void *cpyfrom;
    size_t cpylen;

    if (tolen>fromlen) {
	memset (to, fill, tolen-fromlen);
	cpyto = (char *)to + tolen - fromlen;
	cpylen = fromlen;
	cpyfrom = from;
    } else {
	cpyto = to;
	cpylen = tolen;
	cpyfrom = (char *)from + fromlen - tolen;
    }
    memcpy (cpyto, cpyfrom, cpylen);
    return to;
}

void *a_mvcl_sql (size_t tolen, void *to,
		  int indicator, const void *varfrom, char fill)
{
    size_t fromlen, len;
    const char *from;

    if (indicator != VALID_INDICATOR) {
	memset (to, fill, tolen);
    } else {
	fromlen = *(unsigned short *)varfrom;
	from = (char *)varfrom +2;
	len = fromlen;
	if (len>tolen) len= tolen;
	memcpy (to, from, len);
	if (tolen>len) memset ((char *)to+len, fill, tolen-len);
    }
    return to;
}

void *a_mvcr_sql (size_t tolen, void *to,
		  int indicator, const void *varfrom, char fill)
{
    size_t fromlen, len;
    const char *from;

    if (indicator != VALID_INDICATOR) {
	memset (to, fill, tolen);
    } else {
	fromlen = *(unsigned short *)varfrom;
	from = (char *)varfrom +2;
	len = fromlen;
	if (len>tolen) len= tolen;
	memcpy ((char *)to + (tolen-len), from, len);
	if (tolen>len) memset (to, fill, tolen-len);
    }
    return to;
}

void *a_sql_mvcl (void *tovarchar, short *toind,
		  size_t fromlen, const void *from, char fill)
{
    const char *f;
    short retind;

    f= from;
    while (fromlen>0 && f[fromlen-1]==fill) {
	--fromlen;
    }
    if (fromlen==0) { /* Üres */
	retind= NULL_INDICATOR;
	*(unsigned short *)tovarchar= 0;
    } else {
	retind= VALID_INDICATOR;
	*(unsigned short *)tovarchar= fromlen;
	memcpy ((char *)tovarchar+2, f, fromlen);
    }
    if (toind) *toind = retind;
    return tovarchar;
}

void *a_sql_mvcr (void *tovarchar, short *toind,
		  size_t fromlen, const void *from, char fill)
{
    const char *f;
    short retind;

    f= from;
    while (fromlen>0 && f[0]==fill) {
	--fromlen;
	++f;
    }
    if (fromlen==0) { /* Üres */
	retind= NULL_INDICATOR;
	*(unsigned short *)tovarchar= 0;
    } else {
	retind= VALID_INDICATOR;
	*(unsigned short *)tovarchar= fromlen;
	memcpy ((char *)tovarchar+2, f, fromlen);
    }
    if (toind) *toind = retind;
    return tovarchar;
}

void *a_sql_mvcl_x (size_t tosize,  void *tovarchar, short *toind,
		    size_t fromlen, const void *from, char fill)
{
    void *p;

    if (tosize<2) exit (-1);
    if (fromlen > tosize-2) fromlen= tosize-2;

    p= a_sql_mvcl (tovarchar, toind, fromlen, from, fill);
    return p;
}

void *a_sql_mvcr_x (size_t tosize, void *tovarchar, short *toind,
		    size_t fromlen, const void *from, char fill)
{
    void *p;

    if (tosize<2) exit (-1);
    if (fromlen > tosize-2) {
	*(char *)&from += fromlen - (tosize-2);
	fromlen= tosize-2;
    }

    p= a_sql_mvcr (tovarchar, toind, fromlen, from, fill);
    return p;
}

#endif

void *a_mvcl_c (size_t tolen, void *to,
		const void *fromcstring, char fill)
{
    size_t fromlen;

    if (fromcstring) fromlen = strnlen (fromcstring, tolen);
    else	     fromlen = 0;

    return a_mvcl (tolen, to, fromlen, fromcstring, fill);
}

void *a_mvcr_c (size_t tolen, void *to,
		const void *fromcstring, char fill)
{
    size_t fromlen;

    if (fromcstring) fromlen = strnlen (fromcstring, tolen);
    else	     fromlen = 0;

    return a_mvcr (tolen, to, fromlen, fromcstring, fill);
}

void *a_c_mvcl (void *tocstring,
		size_t fromlen, const void *from, char fill)
{
    const char *f;
    char *t;

    f = from;
    t = tocstring;
    while (fromlen>0 && f[fromlen-1]==fill) {
	--fromlen;
    }
    memcpy (t, f, fromlen);
    t [fromlen] = '\0';
    return t;
}

void *a_c_mvcr (void *tocstring,
		size_t fromlen, const void *from, char fill)
{
    const char *f;
    char *t;

    f = from;
    t = tocstring;
    while (fromlen>0 && f[0]==fill) {
	--fromlen;
	++f;
    }
    memcpy (t, f, fromlen);
    t [fromlen] = '\0';
    return t;
}

int a_sql_mvcl_y (size_t tosize,  void *tovarchar, short *toind,
		  size_t fromlen, const void *from)
{
    short retind;

    if (tosize<2) exit (-1);
    if (fromlen > tosize-2) fromlen= tosize-2;

    if (from==NULL)     fromlen= 0;
    else if (fromlen>0) fromlen= strntrim (from, fromlen);

    if (fromlen==0) { /* Üres */
	retind= NULL_INDICATOR;
	*(unsigned short *)tovarchar= 0;

    } else {
	retind= VALID_INDICATOR;
	*(unsigned short *)tovarchar= fromlen;
	memcpy ((char *)tovarchar+2, from, fromlen);
    }
    if (toind) *toind = retind;

    return retind;
}

/* _z: a 'tosize' netto ertendo: sizeof (myvarchar.arr) */
int a_sql_mvcl_z (size_t nettosize, void *tovarchar, short *toind,
                  size_t fromlen, const void *from)
{
    short retind;

    if (fromlen>nettosize) fromlen= nettosize;
    if (from==NULL) fromlen= 0;
    else	    fromlen= strntrim (from, fromlen);

    if (fromlen==0) { /* Üres */
	retind= NULL_INDICATOR;
	*(unsigned short *)tovarchar= 0;

    } else {
	retind= VALID_INDICATOR;
	*(unsigned short *)tovarchar= fromlen;
	memcpy ((char *)tovarchar+2, from, fromlen);
    }
    if (toind) *toind = retind;

    return retind;
}

size_t a_mvcl_y (size_t tolen,   void *to,
		 size_t fromlen, const void *from)
{
    if (from==NULL)     fromlen= 0;
    else if (fromlen>0) fromlen= strntrim (from, fromlen);

    a_mvcl (tolen, to, fromlen, from, ' ');
    return fromlen;
}

void a_sql_sql (void *tovarchar, short *toind, const void *fromvarchar, int fromind)
{
    if (fromind==NULL_INDICATOR ||
	fromvarchar==NULL ||
	*(unsigned short *)fromvarchar==0) {
	if (toind) *toind= NULL_INDICATOR;
	if (tovarchar) *(short *)tovarchar= 0;

    } else {
	if (toind) *toind= VALID_INDICATOR;
	if (tovarchar) {
	    unsigned fromlen;
	    const char *from;
	    char *to;

	    fromlen= *(unsigned short *)fromvarchar;
	    from= (const char *)fromvarchar + 2;
	    *(unsigned short *)tovarchar = fromlen;
	    to= (char *)tovarchar + 2;
	    memcpy (to, from, fromlen);
	}
    }
}

int a_sql_bd (size_t tomax, void *tovarchar, short *toind,
              const struct ConstBuffData *from)
{
    if (!from || from->len==0 || tomax==0) {
	if (toind) *toind= NULL_INDICATOR;
	if (tovarchar) *(uint16_t *)tovarchar= 0;

    } else {
	size_t outlen;

	if (toind) *toind= VALID_INDICATOR;
	outlen= from->len;
	if (outlen>tomax) outlen= tomax;
	if (tovarchar) {
	    *(uint16_t *)tovarchar= outlen;
	    memcpy ((char *)tovarchar + 2, from->ptr, outlen);
	}
    }
    return 0;
}

int a_bd_sql (BuffData *to, int indicator, const void *varfrom)
{
    return a_bd_sql_mf (to, indicator, varfrom, &DefErrMemFun);
}

int a_bd_sql_mf (BuffData *to, int indicator, const void *varfrom, const err_memory_fun *mf)
{
    unsigned vlen;
    int rc= 0;

    if (indicator== NULL_INDICATOR ||
        varfrom==NULL ||
        (vlen= *(unsigned short *)varfrom)==0) {
        to->ptr= NULL;
        to->len= 0;
    } else {
        to->ptr= mf==NULL? emalloc(vlen): mf->mf_malloc(mf->mf_context, vlen);
        if (to->ptr==NULL) {
            to->len= 0;
            rc= -1;
        } else {
            to->len= vlen;
            memcpy(to->ptr, (char *)varfrom+2, vlen);
        }
    }
    return rc;
}

int a_sql_c (size_t tomax, void *tovarchar, short *toind,
             const char *from)
{
    size_t trimlen;

    trimlen= from? strntrim (from, tomax): 0;
    if (toind) *toind= trimlen? VALID_INDICATOR: NULL_INDICATOR;
    if (tovarchar) {
	*(uint16_t *)tovarchar= (unsigned short)trimlen;
	if (trimlen) {
	    memcpy ((char *)tovarchar + 2, from, trimlen);
	}
    }
    return 0;
}

/* egy újabb verzió: 'strntrim'-et hív; 'tomax' nettó:

    char strbuff [12] = "Innen";
    varchar myvarchar [33];	-- ide --

    a_sql_maxc (sizeof myvarchar.arr, &myvarchar, &myind, sizeobj (strbuff));
*/
int a_sql_maxc (size_t tomax, void *tovarchar, short *toind,
		size_t frommax, const char *from)
{
    size_t maxlen;
    size_t trimlen;

    maxlen= frommax;
    if (tomax<maxlen) maxlen= tomax;
    trimlen= from? strntrim (from, maxlen): 0;
    
    if (toind) *toind= trimlen? VALID_INDICATOR: NULL_INDICATOR;

    if (tovarchar) {
	*(uint16_t *)tovarchar= (unsigned short)trimlen;
	if (trimlen) {
	    memcpy ((char *)tovarchar + 2, from, trimlen);
	}
    }
    return 0;
}
