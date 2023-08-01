/* percblt.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include "defs.h"
#include "strvaker.h"
#include "a_value.h"

#include "perec.h"

StaticBD (Nlen,    "LEN");
StaticBD (Nstrpos, "STRPOS");
StaticBD (Nsubstr, "SUBSTR");
StaticBD (Nstrcat, "STRCAT");
StaticBD (Nput,    "PUT");
StaticBD (Nfput,   "FPUT");
StaticBD (Nmatch,  "MATCH");
StaticBD (Nvalue,  "VALUE");
StaticBD (Nunpack, "UNPACK");
StaticBD (Ntrim,   "TRIM");

static int BLen    (Value *retval, int argc, Value *args);
static int BStrPos (Value *retval, int argc, Value *args);
static int BSubStr (Value *retval, int argc, Value *args);
static int BStrCat (Value *retval, int argc, Value *args);
static int BPut    (Value *retval, int argc, Value *args);
static int BFPut   (Value *retval, int argc, Value *args);
static int BMatch  (Value *retval, int argc, Value *args);
static int BValue  (Value *retval, int argc, Value *args);
static int BUnpack (Value *retval, int argc, Value *args);
static int BTrim   (Value *retval, int argc, Value *args);

static VType TypeS [1]	 = {P_STRING};
static VType TypeA [1]	 = {P_ANY};
static VType TypeNA [2]   = {P_NUMBER, P_ANY};
static VType TypeSN [2]   = {P_STRING, P_NUMBER};
static VType TypeNS [2]   = {P_NUMBER, P_STRING};
static VType TypeSS [2] = {P_STRING, P_STRING};
static VType TypeSSN [3] = {P_STRING, P_STRING, P_NUMBER};
static VType TypeSNN [3] = {P_STRING, P_NUMBER, P_NUMBER};

void BltInit (void)
{
    Function *f;

    f = FunNew (&Nlen);
    f->rtype = P_NUMBER;    /* returns NUMBER */
    f->bltin = BLen;
    f->minargc = 1;
    f->maxargc = 1;
    f->atypno  = 1;
    f->atype = TypeS;	    /* STRING argument */

    f = FunNew (&Nstrpos);
    f->rtype = P_NUMBER;    /* returns NUMBER */
    f->bltin = BStrPos;
    f->minargc = 2;
    f->maxargc = 3;
    f->atypno  = 3;
    f->atype = TypeSSN;     /* arguments: STRING, STRING [,NUMBER] */

    f = FunNew (&Nsubstr);
    f->rtype = P_STRING;    /* returns STRING */
    f->bltin = BSubStr;
    f->minargc = 2;
    f->maxargc = 3;
    f->atypno  = 3;
    f->atype = TypeSNN;     /* arguments: STRING, NUMBER [,NUMBER] */

    f = FunNew (&Nstrcat);
    f->rtype = P_STRING;    /* returns STRING */
    f->bltin = BStrCat;
    f->minargc = 1;
    f->maxargc = 256;
    f->atypno  = 1;
    f->atype = TypeS;	    /* arguments: STRING, ... */

    f = FunNew (&Nput);
    f->rtype = P_NUMBER;    /* returns NUMBER */
    f->bltin = BPut;
    f->minargc = 0;
    f->maxargc = 256;
    f->atypno  = 1;
    f->atype = TypeA;	    /* arguments: ANY, ... */

    f = FunNew (&Nfput);
    f->rtype = P_NUMBER;    /* returns NUMBER */
    f->bltin = BFPut;
    f->minargc = 1;
    f->maxargc = 256;
    f->atypno  = 2;
    f->atype = TypeNA;	    /* arguments: NUMBER,ANY, ... */

    f = FunNew (&Nmatch);
    f->rtype = P_NUMBER;    /* returns NUMBER */
    f->bltin = BMatch;
    f->minargc = 2;
    f->maxargc = 2;
    f->atypno  = 2;
    f->atype = TypeSS;	    /* arguments: STRING, STRING */

    f = FunNew (&Nvalue);
    f->rtype = P_NUMBER;    /* returns NUMBER */
    f->bltin = BValue;
    f->minargc = 1;
    f->maxargc = 2;
    f->atypno  = 2;
    f->atype = TypeSN;	    /* arguments: STRING, NUMBER */

    f = FunNew (&Nunpack);
    f->rtype = P_STRING;    /* returns STRING */
    f->bltin = BUnpack;
    f->minargc = 2;
    f->maxargc = 2;
    f->atypno  = 2;
    f->atype = TypeNS;	    /* arguments: NUMBER, STRING */

    f = FunNew (&Ntrim);
    f->rtype = P_STRING;    /* returns STRING */
    f->bltin = BTrim;
    f->minargc = 1;
    f->maxargc = 1;
    f->atypno  = 1;
    f->atype = TypeS;	   /* argument: STRING */
}

static int BLen (Value *retval, int argc, Value *args)
{
    retval->vtype = P_NUMBER;
    retval->u.i = args[0].u.b.len;
    return 0;
}

static int BStrPos (Value *retval, int argc, Value *args)
{
    struct strvaker_par svp;
    int offs, retpoz;

    svp.text	= args[0].u.b.ptr;
    svp.textlen = args[0].u.b.len;
    svp.item	= args[1].u.b.ptr;
    svp.itemlen = args[1].u.b.len;

    offs = 0;

    if (argc>=3) { /* van kezdopozicio */
	offs = args[2].u.i - 1;
	if (offs < 0) offs = 0; /* de jo vicc volt */
    }

    if (offs + svp.itemlen > svp.textlen) { retpoz = 0; goto VEGE; }
    if (svp.itemlen == 0) { retpoz = offs + 1; goto VEGE; }
    svp.text	+= offs;
    svp.textlen -= offs;

    strvaker (&svp);
    if (svp.retptr != NULL) retpoz = (svp.retptr - svp.text) + 1 + offs;
    else		    retpoz = 0;

VEGE:
    retval->vtype = P_NUMBER;
    retval->u.i = retpoz;
    return 0;
}

static int BSubStr (Value *retval, int argc, Value *args)
{
    int rc;
    char *ptr;
    int len, offs, sublen;

    ptr = args[0].u.b.ptr;
    len = args[0].u.b.len;
    offs = args[1].u.i;

    if (offs <= 0) offs = 0;
    else	     --offs;

    if (argc>=3) {
	sublen = args[2].u.i;
	if (sublen < 0) goto ERRET;
    } else sublen = len;

    if (offs > len) offs = sublen = 0;	/* or goto ERRET; */
    else if (offs + sublen > len) sublen = len - offs;

    if (sublen==0) goto EMPTY;

    retval->vtype = P_STRING;
    retval->u.b.len = sublen;
    retval->u.b.ptr = ptr + offs;
    retval.mem = args[0].mem;
    MemUse (&retval->mem);
    return 0;

ERRET:
    rc = -1;
    goto EMPRET;

EMPTY:
    rc = 0;

EMPRET:
    retval->vtype = P_STRING;
    retval->u.b.len = 0;
    retval->u.b.ptr = "";
    retval->mem = NULL;
    return rc;
}

static int BStrCat (Value *retval, int argc, Value *args)
{
    char *ptr, *wptr;
    int i, len, sumlen;
    Memory *m;
    int rc;

    sumlen = 0;
    for (i=0; i<argc; ++i) {
	len = args[i].u.b.len;
	if (len<0) goto ERRET;
	sumlen += len;
    }
    if (sumlen==0) goto EMPTY;

    m = MemGet (sumlen);
    ptr = MemAddr (m);

    for (i=0, wptr= ptr; i<argc; ++i) {
	len = args[i].u.b.len;
	if (len>0) {
	    memcpy (wptr, args[i].u.b.ptr, len);
	    wptr += len;
	}
    }

    retval->vtype = P_STRING;
    retval->u.b.len = sumlen;
    retval->u.b.ptr = ptr;
    retval->mem = m;
    return 0;

ERRET:
    rc = -1;
    goto EMPRET;

EMPTY:
    rc = 0;

EMPRET:
    retval->vtype = P_STRING;
    retval->u.b.len = 0;
    retval->u.b.ptr = "";
    retval->mem = NULL;
    return rc;
}

static int BPut (Value *retval, int argc, Value *args)
{
    int i, bytec, bytes;
    int rc;

    rc = 0; bytes = 0;
    for (i=0; i<argc; ++i) {
	bytec = ValuePrint (&args[i], 0);
	if (bytec< 0) { bytes = -1; goto RET; }
	bytes += bytec;
    }
    putc ('\n', stdout);

RET:
    retval->vtype = P_NUMBER;
    retval->u.i   = bytes;
    retval->mem = NULL;
    return rc;
}

static int BFPut (Value *retval, int argc, Value *args)
{
    int i;
    int rc;
    char *ptr, *wptr;

    ptr = emalloc (32768+4);
    rc = 0;
    wptr = ptr+4;
    for (i=1; i<argc; ++i) {
	if (args[i].vtype == P_NUMBER) {
	    wptr += sprintf (wptr, "%d", args[i].u.i);
	} else {
	    memcpy (wptr, args[i].u.b.ptr, args[i].u.b.len);
	    wptr += args[i].u.b.len;
	}
    }
    *(unsigned short *)ptr = wptr - ptr;
    OutPut (args[0].u.i, ptr);
    free (ptr);

    retval->vtype = P_NUMBER;
    retval->u.i   = wptr - ptr - 4;
    retval->mem = NULL;
    return rc;
}

static int BMatch  (Value *retval, int argc, Value *args)
{
    regex_t re;
    int  rc, retv;
    char error [256];
    int len;
    char *ptr, save;

    ptr = args[1].u.b.ptr;
    len = args[1].u.b.len;
    save = ptr[len];
    ptr[len] = 0;
    rc = regcomp (&re, ptr, REG_EXTENDED | REG_NOSUB);
    ptr[len] = save;
    if (rc) {
	fprintf (stderr, "Error %d in regcomp '%.*s'\n",
		 rc, len, ptr);
	regerror (rc, &re, error, sizeof (error));
	fprintf (stderr, "%s\n", error);
	exit (8);
    }

    ptr = args[0].u.b.ptr;
    len = args[0].u.b.len;
    save = ptr[len];
    ptr[len] = 0;
    rc = regexec (&re, ptr, 0, NULL, 0);
    ptr[len] = save;
    if (rc==0) retv = 1;
    else if (rc==REG_NOMATCH) retv = 0;
    else {
	fprintf (stderr, "Error %d in regexec ('%.*s','%.*s')\n",
		 rc, args[1].u.b.len, args[1].u.b.ptr,
		 len, ptr);
	regerror (rc, &re, error, sizeof (error));
	fprintf (stderr, "%s\n", error);
	exit (8);
    }
    regfree (&re);

    retval->vtype = P_NUMBER;
    retval->u.i   = retv;
    retval->mem = NULL;
    return 0;
}

static int BValue (Value *retval, int argc, Value *args)
{
    int base;
    int rc;
    ValuePar vp;

    rc = 0;

    if (argc>1) base = args[1].u.i;
    else	base = 10;

    vp.BASE = base;
    vp.LEN = args[0].u.b.len;
    vp.ADDR = args[0].u.b.ptr;

    rc = A_Value (&vp);

    retval->vtype = P_NUMBER;
    retval->u.i   = vp.BASE;
    retval->mem = NULL;
    return rc;
}

typedef void UnpackFun (void *vlaza_out, void *vtomor_in);

static int BUnpack (Value *retval, int argc, Value *args)
{
    char vtomor [32768], *vlaza;
    Memory *m;
    UnpackFun *funptr;

    memcpy (vtomor+4, args[1].u.b.ptr, args[1].u.b.len);
    *(unsigned short *)vtomor = 4 + args[1].u.b.len;

    m= MemGet (32768);

    vlaza = MemAddr (m);

    funptr = (UnpackFun *)(args[0].u.i);
    (*funptr)(vlaza, vtomor);

    retval->vtype = P_STRING;
    retval->u.b.len = *(unsigned short *)vlaza -4;
    retval->u.b.ptr = vlaza+4;
    retval->mem = m;

    return 0;
}

static int BTrim (Value *retval, int argc, Value *args)
{
    char *ptr;
    int len;

    ptr = args[0].u.b.ptr;
    len = args[0].u.b.len;

    while (len>0 && ptr[len-1]==' ') --len;

    retval->vtype = P_STRING;
    retval->u.b.len = len;
    retval->u.b.ptr = ptr;
    retval->mem = args[0].mem;

    MemUse (&retval->mem);
    return 0;
}
