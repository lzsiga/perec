/* buffdata.c */

#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "defs.h"
#include "buffdata.h"
#include "tilog.h"

#if (defined(_Windows) || defined(_WIN32)) && !defined(strcasecmp)
    #define strcasecmp	stricmp
    #define strncasecmp strnicmp
#endif

DLL_PUBLIC (int, BuffDataCmpX) (const ConstBuffData *a, const ConstBuffData *b,
	BuffData *a_comm)
{
    size_t i, cmplen;
    int def, cmp;
    const char *p, *q;

    if (a->len > b->len)      def = 1,	cmplen = b->len;
    else if (a->len < b->len) def = -1, cmplen = a->len;
    else		      def = 0,	cmplen = a->len;

    p= a->ptr;
    q= b->ptr;
    for (i= (size_t)-1; ++i<cmplen && p[i]==q[i]; );
    /* kilépés: 0<=i==cmplen -> nincs különbség */
    /*		0<=i<cmplen  -> p[i] != q[i]	*/

    if (i<cmplen) cmp= (p[i]>q[i]) - (p[i]<q[i]);
    else 	  cmp= 0;

    if (a_comm) {
	a_comm->ptr= (char *)a->ptr;
	a_comm->len= i;
    }

    if (cmp==0) cmp = def;
    return cmp;
}

DLL_PUBLIC (int, BuffDataCmp) (const ConstBuffData *a, const ConstBuffData *b)
{
    size_t cmplen;
    int def, cmp;

    if (a->len > b->len)      def = 1,	cmplen = b->len;
    else if (a->len < b->len) def = -1, cmplen = a->len;
    else		      def = 0,	cmplen = a->len;

    if (cmplen == 0) cmp = 0;
    else	     cmp = memcmp (a->ptr, b->ptr, cmplen);

    if (cmp==0) cmp = def;
    return cmp;
}

DLL_PUBLIC (int, BuffDataCmpI) (const ConstBuffData *a, const ConstBuffData *b)
{
    size_t cmplen;
    int def, cmp;

    if (a->len > b->len)      def = 1,	cmplen = b->len;
    else if (a->len < b->len) def = -1, cmplen = a->len;
    else		      def = 0,	cmplen = a->len;

    if (cmplen == 0) cmp = 0;
    else	     cmp = strncasecmp (a->ptr, b->ptr, cmplen);

    if (cmp==0) cmp = def;
    return cmp;
}

DLL_PUBLIC (int, BuffDataEq) (const ConstBuffData *a, const ConstBuffData *b)
{
    if (a->len != b->len)     return 0;   /* not equal */
    if (a->len == 0)	      return 1;   /* equal */
    return		      memcmp (a->ptr, b->ptr, a->len) == 0;
}

DLL_PUBLIC (int, BuffDataEqI) (const ConstBuffData *a, const ConstBuffData *b)
{
    if (a->len != b->len)   return 0;
    if (a->len == 0)	    return 1;
    return		    strncasecmp (a->ptr, b->ptr, a->len) == 0;
}

DLL_PUBLIC (int, BuffDataBeg) (const ConstBuffData *data, const ConstBuffData *beg)
{
    if (data->len < beg->len) return 0; /* not match */
    return memcmp (data->ptr, beg->ptr, beg->len) == 0;
}

DLL_PUBLIC (int, BuffDataBegI) (const ConstBuffData *data, const ConstBuffData *beg)
{
    if (data->len < beg->len) return 0;
    return strncasecmp (data->ptr, beg->ptr, beg->len) == 0;
}

DLL_PUBLIC (int, BuffDataBegX) (int popt, const ConstBuffData *blong, const ConstBuffData *bshort, char sep)
{
    int retval;
    ConstBuffData bdiff;
    int sepf= popt&BBX_OPT_SEP;
    bdiff.ptr = blong->ptr + bshort->len;
    bdiff.len = blong->len - bshort->len;

    if (bshort->len > blong->len) {
	retval= 0;

    } else if (bshort->len == blong->len) {
	retval= (popt&BBX_OPT_EQ)==BBX_OPT_EQ_OK;

    } else if (sepf==BBX_OPT_SEP_NO) {
	retval= 1;

    } else if (bdiff.ptr[0]!=sep) {
	retval= 0;

    } else if (sepf==BBX_OPT_SEP_Y) {
	retval= 1;

    } else if (bdiff.len<2) {
	retval= 0;

    } else if (sepf==BBX_OPT_SEP_Y2) {
	retval= 1;

    } else {
	retval= bdiff.ptr[1] != sep;
    }

    return retval;
}

DLL_PUBLIC (int, BuffDataEndX) (int popt, const ConstBuffData *blong, const ConstBuffData *bshort, char sep)
{
    int retval;
    ConstBuffData bdiff;
    int sepf= popt&BBX_OPT_SEP;
    bdiff.ptr = blong->ptr;
    bdiff.len = blong->len - bshort->len;

    if (bshort->len > blong->len) {
	retval= 0;

    } else if (bshort->len == blong->len) {
	retval= (popt&BBX_OPT_EQ)==BBX_OPT_EQ_OK;

    } else if (sepf==BBX_OPT_SEP_NO) {
	retval= 1;

    } else if (bdiff.ptr[bdiff.len-1]!=sep) {
	retval= 0;

    } else if (sepf==BBX_OPT_SEP_Y) {
	retval= 1;

    } else if (bdiff.len<2) {
	retval= 0;

    } else if (sepf==BBX_OPT_SEP_Y2) {
	retval= 1;

    } else {
	retval= bdiff.ptr[bdiff.len-2] != sep;
    }

    return retval;
}

DLL_PUBLIC (int, BuffDataEnd) (const ConstBuffData *data, const ConstBuffData *end)
{
    if (data->len < end->len) return 0; /* not match */
    return memcmp (data->ptr + data->len - end->len,
		   end->ptr, end->len) == 0;
}

DLL_PUBLIC (int, BuffDataEndI) (const ConstBuffData *data, const ConstBuffData *end)
{
    if (data->len < end->len) return 0;
    return strncasecmp (data->ptr + data->len - end->len,
		   end->ptr, end->len) == 0;
}

DLL_PUBLIC (void, BuffAppend) (Buffer *into, const ConstBuffData *from)
{
    size_t sumlen;

    if (! from->len) return;
    sumlen = into->len + from->len;
    if (sumlen > into->maxlen) BuffSecX (into, from->len);

    memcpy (into->ptr + into->len, from->ptr, from->len);
    into->len += from->len;
}

DLL_PUBLIC (void, BuffAppend_M) (Buffer *into, const ConstBuffData *from,
    const struct memory_fun *memfun)
{
    size_t sumlen;

    if (! from->len) return;
    sumlen = into->len + from->len;
    if (sumlen > into->maxlen) BuffSec_M (into, from->len, memfun);

    memcpy (into->ptr + into->len, from->ptr, from->len);
    into->len += from->len;
}

DLL_PUBLIC (void, BuffAppendLP) (Buffer *into, size_t len, const void *p)
{
    ConstBuffData from;

    if (len) {
	from.len= len;
	from.ptr= (const char *)p;
	BuffAppend (into, &from);
    }
}

void BuffAppendLP_M (DynBuffer *into, size_t len, const void *p,
    const struct memory_fun *memfun)
{
    ConstBuffData from;

    if (len) {
	from.len= len;
	from.ptr= (const char *)p;
	BuffAppend_M (into, &from, memfun);
    }
}

DLL_PUBLIC (void, BuffAppendS) (Buffer *into, const char *s)
{
    ConstBuffData from;

    if (s && *s) {
	from.len= strlen (s);
	from.ptr= s;
	BuffAppend (into, &from);
    }
}

DLL_PUBLIC (void, BuffAppendS_M) (DynBuffer *into, const char *s,
    const struct memory_fun *memfun)
{
    ConstBuffData from;

    if (s && *s) {
	from.len= strlen (s);
	from.ptr= s;
	BuffAppend_M (into, &from, memfun);
    }
}

DLL_PUBLIC (void, BuffInsert) (Buffer *into, const ConstBuffData *from)
{
    size_t sumlen;

    sumlen = into->len + from->len;
    if (sumlen>into->maxlen) {
	into->ptr = erealloc (into->ptr, sumlen);
	into->maxlen = sumlen;
    }
    if (into->len>0 && from->len>0) {
	memmove (into->ptr + from->len, into->ptr, into->len);
    }
    if (from->len) {
	memcpy (into->ptr, from->ptr, from->len);
    }
    into->len += from->len;
}

DLL_PUBLIC (void, BuffInsertAt) (Buffer *into, size_t tooffs,
		   const ConstBuffData *from)
{
    size_t sumlen;

    sumlen = into->len + from->len;
    if (sumlen>into->maxlen) {
	into->ptr = erealloc (into->ptr, sumlen);
	into->maxlen = sumlen;
    }
    if (tooffs>into->len) tooffs= into->len;

    if ((into->len - tooffs)>0 && from->len>0) {
	memmove (into->ptr + tooffs + from->len,
		 into->ptr + tooffs,
		 into->len - tooffs);
    }
    if (from->len) {
	memcpy (into->ptr + tooffs, from->ptr, from->len);
    }
    into->len += from->len;
}

DLL_PUBLIC (void, BuffCut) (Buffer *from, size_t cutoff, size_t cutlen)
{
    if (cutoff > from->len);
    else if (cutlen >= from->len-cutoff) {
	from->len = cutoff;
    } else {
	memmove (from->ptr + cutoff, from->ptr + cutoff + cutlen,
		 from->len - cutoff - cutlen);
	from->len -= cutlen;
    }
}

DLL_PUBLIC (void, BuffSecure) (DynBuffer *into, size_t minimum)
{
    BuffSec_M (into, minimum, &DefErrMemFun);
}

DLL_PUBLIC (void, BuffSecX) (DynBuffer *into, size_t minimum)
{
    BuffSec_M (into, minimum, &DefErrMemFun);
}

DLL_PUBLIC (void, BuffSec_M) (DynBuffer *into, size_t minimum, const err_memory_fun *memfun)
{
    size_t freesize, reqsize;

    if (!memfun) memfun= &DefErrMemFun;
    freesize= into->maxlen - into->len;
    if (freesize >= minimum) return;

    reqsize = minimum - freesize;
    if (reqsize < into->maxlen) reqsize= into->maxlen; /* aggressive;) */
    into->maxlen += reqsize;
    into->ptr = memfun->mf_realloc (memfun->mf_context, into->ptr, into->maxlen);
}

DLL_PUBLIC (int, LimBuffSec) (int opt, LimBuffer *into, size_t minimum)
{
    return LimBuffSec_M (opt, into, minimum, &DefErrMemFun);
}

DLL_PUBLIC (int, LimBuffSec_M) (int opt, LimBuffer *into, size_t minimum,
    const err_memory_fun *memfun)
{
    size_t reqsize, freesize, maxalloc;
    int rc= 0;

    if (! IsLimBufferSane (into)) {
	rc= -1;
	goto RETURN;
    }

    freesize= into->maxlen- into->len;
    maxalloc= into->limit - into->maxlen;

    if (minimum <= freesize) {
	rc= 0;
	goto RETURN;
    } else if (maxalloc==0) {
	rc= 1;
	goto RETURN;
    }
    
    reqsize= minimum - freesize;

    if (reqsize > maxalloc) {
	rc= 1;
	if ((opt&1)==0) goto RETURN; 			/* don't allocate at all */
	else reqsize = maxalloc;			/* allocate less */
    }

    if (reqsize < into->maxlen) reqsize= into->maxlen;	/* aggressive;) */
    if (reqsize > maxalloc)     reqsize= maxalloc;	/* degressive;) */

    into->maxlen += reqsize;
    into->ptr = memfun->mf_realloc (memfun->mf_context, into->ptr, into->maxlen);

RETURN:
    return rc;
}

DLL_PUBLIC (void, BuffCopy) (Buffer *into, const ConstBuffData *from)
{
    into->len= 0;

    if (into->maxlen < from->len) BuffSecX (into, from->len);
    memcpy (into->ptr, from->ptr, from->len);
    into->len = from->len;
}

DLL_PUBLIC (void, BuffCopyZ) (Buffer *into, const ConstBuffData *from)
{
    into->len= 0;

    if (into->maxlen < from->len+1) BuffSecX (into, from->len+1);
    if (from->len) memcpy (into->ptr, from->ptr, from->len);
    into->ptr[from->len] = '\0';
    into->len = from->len;
}

DLL_PUBLIC (void, BuffCopyS) (Buffer *into, const char *from)
{
    ConstBuffData bd;

    bd.ptr= from;
    if (from && *from) bd.len= strlen (from);
    else	       bd.len= 0;
    BuffCopyZ (into, &bd);
}

DLL_PUBLIC (void, BuffCutback) (Buffer *b, size_t newmaxlen /* >=0 */)
{
    BuffCutback_M (b, newmaxlen, &DefErrMemFun);
}

DLL_PUBLIC (void, BuffCutback_M) (DynBuffer *b, size_t newmaxlen, const err_memory_fun *memfun)
{
    if (b->maxlen==newmaxlen) return;

    b->maxlen = newmaxlen;
    if (b->len > newmaxlen) b->len= newmaxlen;

/* 20110824: Az 'Electric Fence' szerint a realloc (ptr, 0)
   valószínûleg hiba. Téved ugyan, de inkább tegyünk ide
   egy elágazást: ha newmaxlen==0, akkor free-t hívunk
 */
    if (newmaxlen==0) {
	memfun->mf_free (memfun->mf_context, b->ptr);
	b->ptr= NULL;

    } else {
	b->ptr = memfun->mf_realloc (memfun->mf_context,b->ptr, newmaxlen);
    }
}

DLL_PUBLIC (void, LimBuffCutback_M) (LimBuffer *b, size_t newmaxlen, const err_memory_fun *memfun)
{
    if (!memfun) memfun= &DefErrMemFun;

    if (newmaxlen > b->limit || newmaxlen > b->maxlen) {
	TiLogF (stderr, 0, "buffer.LimBuffCutback_M(%u): *** length error (limit=%u,len=%u,maxlen=%u"
	       , (unsigned)newmaxlen, (unsigned)b->limit
	       , (unsigned)b->len,    (unsigned)b->maxlen
	       );
	exit (34);
    }
    BuffCutback_M ((DynBuffer *)b, newmaxlen, memfun);
}

DLL_PUBLIC (void, BuffDup) (BuffData *into, const ConstBuffData *from)
{
    if (from && from->len) {
	into->ptr = emalloc (from->len);
	memcpy (into->ptr, from->ptr, from->len);
    } else {
	into->ptr= NULL;
    }
    into->len = from->len;
}

DLL_PUBLIC (void, BuffDup_M) (BuffData *into, const ConstBuffData *from,
    const struct memory_fun *memfun)
{
    if (!memfun) memfun= &DefErrMemFun;

    if (from && from->len) {
	into->ptr = memfun->mf_malloc (memfun->mf_context, from->len);
	memcpy (into->ptr, from->ptr, from->len);
    } else {
	into->ptr= NULL;
    }
    into->len = from->len;
}

DLL_PUBLIC (void, BuffDupZ) (BuffData *into, const ConstBuffData *from)
{
    into->ptr = emalloc (from->len+1);
    into->len = from->len;
    if (from->len) memcpy (into->ptr, from->ptr, from->len);
    into->ptr [from->len] = '\0';
}

DLL_PUBLIC (void, BuffDupZ_M) (BuffData *into, const ConstBuffData *from,
    const struct memory_fun *memfun)
{
    if (!memfun) memfun= &DefErrMemFun;

    into->ptr = memfun->mf_malloc (memfun->mf_context, from->len+1);
    into->len = from->len;
    if (from->len) memcpy (into->ptr, from->ptr, from->len);
    into->ptr [from->len] = '\0';
}

/* egy lezáró \0-t is tesz a régi szép idõk emlékére */
DLL_PUBLIC (void, BuffDupS) (BuffData *into, const char *from)
{
    ConstBuffData bd;

    bd.ptr= from;
    if (from && *from) {
	bd.len = strlen (from);
    } else {
	bd.len = 0;
    }
    BuffDupZ (into, &bd);
}

/* egy lezáró \0-t is tesz a régi szép idõk emlékére */
DLL_PUBLIC (void, BuffDupS_M) (BuffData *into, const char *from,
    const struct memory_fun *memfun)
{
    ConstBuffData bd;

    bd.ptr= from;
    if (from && *from) {
	bd.len = strlen (from);
    } else {
	bd.len = 0;
    }
    BuffDupZ_M (into, &bd, memfun);
}

DLL_PUBLIC (void, BuffDupLP_M) (BuffData *into, size_t len, const void *from,
    const struct memory_fun *memfun)
{
    ConstBuffData bd= {from, len};
    BuffDup_M (into, &bd, memfun);
}

DLL_PUBLIC (void, BuffDupLPZ_M) (BuffData *into, size_t len, const void *from,
    const struct memory_fun *memfun)
{
    ConstBuffData bd= {from, len};
    BuffDupZ_M (into, &bd, memfun);
}

DLL_PUBLIC (void, BuffRelease)(BuffData *mem)
{
    if (mem) {
	if (mem->ptr) {
	    efree (mem->ptr);
	    mem->ptr= NULL;
	}
	mem->len= 0;
    }
}

DLL_PUBLIC (void, BuffRelease_M)(BuffData *mem, const struct memory_fun *memfun)
{
    if (!memfun) memfun= &DefErrMemFun;

    if (mem) {
	if (mem->ptr) {
	    memfun->mf_free (memfun->mf_context, mem->ptr);
	    mem->ptr= NULL;
	}
	mem->len= 0;
    }
}

DLL_PUBLIC (void, XBD_RelToAbs)(const RelBuffData *from, const void *base, BuffData *to)
{
    to->len= from->len;
    if (from->ptr==(ptrdiff_t)-1) to->ptr= NULL;
    else			  to->ptr= (char *)base + from->ptr;
}

DLL_PUBLIC (void, XBD_AbsToRel)(const ConstBuffData *from, const void *base, RelBuffData *to)
{
    to->len= from->len;
    if (from->ptr==NULL) to->ptr= (ptrdiff_t)-1;
    else		 to->ptr= from->ptr - (char *)base;
}

DLL_PUBLIC (void, BuffAddTermZero) (DynBuffer *b)
{
    if (b->maxlen < b->len+1) BuffSecX (b, 1);
    b->ptr[b->len] = '\0';
}

DLL_PUBLIC (void, BuffAddTermZeroes) (DynBuffer *b, unsigned nzero)
{
    if (!nzero) return; /* very funny */
    if (b->maxlen < b->len+nzero) BuffSecX (b, (b->len+nzero) - b->maxlen);
    memset (b->ptr + b->len, 0, nzero);
}

DLL_PUBLIC (void, BuffAddTermZero_mf) (DynBuffer *b, const err_memory_fun *memfun)
{
    if (b->maxlen < b->len+1) BuffSec_M (b, 1, memfun);
    b->ptr[b->len] = '\0';
}

DLL_PUBLIC (void, BuffAppendChar) (DynBuffer *b, char c)
{
    if (b->maxlen < b->len+1) BuffSecure (b, 1);
    b->ptr[b->len++] = c;
}

DLL_PUBLIC (void, BuffAppendChar_mf) (DynBuffer *b, char c, const err_memory_fun *memfun)
{
    if (b->maxlen < b->len+1) BuffSec_M (b, 1, memfun);
    b->ptr[b->len++] = c;
}

DLL_PUBLIC (void, BuffAddTermZeroes_mf) (DynBuffer *b, unsigned nzero, const err_memory_fun *memfun)
{
    if (!nzero) return; /* very funny */
    if (b->maxlen < b->len+nzero) BuffSec_M (b, (b->len+nzero) - b->maxlen, memfun);
    memset (b->ptr + b->len, 0, nzero);
}

DLL_PUBLIC (int, LimBuffInsertAt) (LimBuffer *into, size_t tooffs,
		   const ConstBuffData *from)
{
    size_t sumlen;

    if (!IsLimBufferSane (into)) return -1;
    if (from==NULL || from->len==0) return 0;

    sumlen = into->len + from->len;

    if (sumlen>into->maxlen) {
	int rc= LimBuffSec_M (0, into, from->len, &DefErrMemFun);
	if (rc) return rc;
    }
    if (tooffs>into->len) tooffs= into->len;

    if ((into->len - tooffs)>0 && from->len>0) {
	memmove (into->ptr + tooffs + from->len,
		 into->ptr + tooffs,
		 into->len - tooffs);
    }
    if (from->len) {
	memcpy (into->ptr + tooffs, from->ptr, from->len);
    }
    into->len += from->len;
    return 0;
}

DLL_PUBLIC (int, LimBuffAppend) (LimBuffer *into, const ConstBuffData *from)
{
    int rc= LimBuffInsertAt (into, into->len, from);
    return rc;
}
