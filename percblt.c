/* percblt.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <time.h>

#include "a_mvcl.h"
#include "defs.h"
#include "strvaker.h"
#include "a_value.h"
#include "szio.h"

#include "perec.h"

StaticBD (Nlen,    "LEN");
StaticBD (Nstrpos, "STRPOS");
StaticBD (Nsubstr, "SUBSTR");
StaticBD (Nstrcat, "STRCAT");
StaticBD (Nput,    "PUT");
StaticBD (Nfput,   "FPUT");
StaticBD (Nmatch,  "MATCH");
StaticBD (Nvalue,  "VALUE");	/*  C'1000' --> 1000 */
StaticBD (Nbinval, "BINVALUE"); /*  X'03E8' --> 1000 */
StaticBD (Nchar,   "CHAR");	/*  1000    --> C'1000' */
StaticBD (Nunpack, "UNPACK");
StaticBD (Ntrim,   "TRIM");
StaticBD (Nopeni,  "OPENI");
StaticBD (Nclose,  "CLOSE");
StaticBD (Nget,    "GET");
StaticBD (Nerr,    "ERR");
StaticBD (Ndate,   "DATE");
StaticBD (Ndatime, "DATETIME");

static int BLen    (Value *retval, int argc, Value *args);
static int BStrPos (Value *retval, int argc, Value *args);
static int BSubStr (Value *retval, int argc, Value *args);
       int BStrCat (Value *retval, int argc, Value *args);
static int BPut    (Value *retval, int argc, Value *args);
static int BFPut   (Value *retval, int argc, Value *args);
static int BMatch  (Value *retval, int argc, Value *args);
static int BValue  (Value *retval, int argc, Value *args);
static int BBinVal (Value *retval, int argc, Value *args);
static int BChar   (Value *retval, int argc, Value *args);
static int BUnpack (Value *retval, int argc, Value *args);
static int BTrim   (Value *retval, int argc, Value *args);
static int BOpenI  (Value *retval, int argc, Value *args);
static int BClose  (Value *retval, int argc, Value *args);
static int BGet    (Value *retval, int argc, Value *args);
static int BErr    (Value *retval, int argc, Value *args);
static int BDate   (Value *retval, int argc, Value *args);
static int BDatime (Value *retval, int argc, Value *args);

static VType TypeN [1]	 = {P_NUMBER};
static VType TypeS [1]	 = {P_STRING};
static VType TypeA [1]	 = {P_ANY};
static VType TypeNA [2]   = {P_NUMBER, P_ANY};
static VType TypeSN [2]   = {P_STRING, P_NUMBER};
static VType TypeNS [2]   = {P_NUMBER, P_STRING};
/* static VType TypeNN [2]  = {P_NUMBER, P_NUMBER}; */
static VType TypeSS [2]  = {P_STRING, P_STRING};
static VType TypeSSN [3] = {P_STRING, P_STRING, P_NUMBER};
static VType TypeSNN [3] = {P_STRING, P_NUMBER, P_NUMBER};
static VType TypeNNS [3] = {P_NUMBER, P_NUMBER, P_STRING};

#define Define(pname,prettype,pfun,pmin,pmax,patypn,patype) \
    f = FunNew (pname);   \
    f->rtype = prettype;  \
    f->bltin = pfun;	  \
    f->minargc = pmin;	  \
    f->maxargc = pmax;	  \
    f->atypno  = patypn;  \
    f->atype = patype;

void BltInit (void)
{
    Function *f;

    Define (&Nlen,    P_NUMBER, BLen, 1, 1, 1, TypeS);
    Define (&Nstrpos, P_NUMBER, BStrPos, 2, 3, 3, TypeSSN);

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

    f = FunNew (&Nbinval);
    f->rtype = P_NUMBER;    /* returns NUMBER */
    f->bltin = BBinVal;
    f->minargc = 1;
    f->maxargc = 1;
    f->atypno  = 1;
    f->atype = TypeS;	    /* argument: STRING */

    f = FunNew (&Nunpack);
    f->rtype = P_STRING;    /* returns STRING */
    f->bltin = BUnpack;
    f->minargc = 2;
    f->maxargc = 2;
    f->atypno  = 2;
    f->atype = TypeNS;	    /* arguments: NUMBER, STRING */

    f = FunNew (&Nchar);
    f->rtype = P_STRING;    /* returns STRING */
    f->bltin = BChar;
    f->minargc = 1;
    f->maxargc = 3;
    f->atypno  = 3;
    f->atype = TypeNNS;     /* arguments: NUMBER, NUMBER, STRING */

    Define (&Ntrim,   P_STRING, BTrim,	1, 1, 1, TypeS);
    Define (&Nopeni,  P_NUMBER, BOpenI, 1, 1, 1, TypeS);
    Define (&Nclose,  P_NUMBER, BClose, 1, 1, 1, TypeN);
    Define (&Nget,    P_STRING, BGet,	1, 1, 1, TypeN);
    Define (&Nerr,    P_NUMBER, BErr,	1, 1, 1, TypeN);

    Define (&Ndate,   P_STRING, BDate,	0, 0, 0, NULL);
    Define (&Ndatime, P_STRING, BDatime,0, 0, 0, NULL);
}

static int BLen (Value *retval, int argc __attribute__((unused)), Value *args)
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

int BStrCat (Value *retval, int argc, Value *args)
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
	    wptr += sprintf (wptr, "%ld", args[i].u.i);
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

static int BMatch (Value *retval, int argc __attribute__((unused)), Value *args)
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
		 rc, (int)args[1].u.b.len, args[1].u.b.ptr,
		 (int)len, ptr);
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

static int BBinVal (Value *retval, int argc __attribute__((unused)), Value *args)
{
    long value;
    int rc, plen;
    const char *p;
    char tmp[4];

    rc = 0;

    plen = args[0].u.b.len;
    p	 = args[0].u.b.ptr;

    if (plen==0) {
	value = 0;
	rc = 0;
    } else if (plen==1) {
	value = *(unsigned char *)p;
	rc = 0;
    } else if (plen==2) {
	value = *(unsigned short *)p;
	rc = 0;

    } else if (plen==3) {
	tmp[0] = 0;
	memcpy (tmp+1, p, 3);
	value = *(unsigned int *)tmp; /* tfh sizeof(int)==4! */
	rc = 0;

    } else if (plen==4) {
	value = *(unsigned int *)p;   /* tfh sizeof(int)==4! */
	rc = 0;

    } else {
	rc = -1;
    }

    retval->vtype = P_NUMBER;
    retval->u.i   = value;
    retval->mem = NULL;
    return rc;
}

typedef void UnpackFun (void *vlaza_out, void *vtomor_in);

static int BChar (Value *retval, int argc, Value *args)
{
    char tmp [64], *p, *q;
    Memory *m;
    int fldlen, len, abslen, sign, sep_sign;
    int fldalign;
    long val, absval;
    int fillchar;

    fldalign= 0;

    val = args[0].u.i;
    if (val<0) {
	sign= 1;
	absval= -val;
    } else {
	sign= 0;
	absval= val;
    }
    abslen = sprintf (tmp, "%ld", absval);
    len = abslen+sign;

    if (argc>=2) {
	fldlen = args[1].u.i;
	if (fldlen<0) {
	    fldlen= -fldlen;
	    fldalign= 1;
	}
	if (fldlen<len) fldlen= len;
    } else {
	fldlen = len;
    }

    if (argc>=3 && args[2].u.b.len>=1) {
	fillchar = args[2].u.b.ptr[0];
    } else {
	fillchar = ' ';
    }

    m= MemGet (fldlen);

    p = MemAddr (m);
    q = p;

    sep_sign = fldlen>len && fldalign==0 && fillchar =='0';

    if (sign && sep_sign) {
	*q++ = '-';
    }
    if (fldlen>len && fldalign==0) {
	memset (q, fillchar, fldlen-len);
	q += fldlen-len;
    }
    if (sign && !sep_sign) {
	*q++ = '-';
    }
    memcpy (q, tmp, abslen);
    q += abslen;

    if (fldlen>len && fldalign!=0) {
	memset (q, fillchar, fldlen-len);
	q += fldlen-len;
    }

    retval->vtype = P_STRING;
    retval->u.b.len = fldlen;
    retval->u.b.ptr = p;
    retval->mem = m;

    return 0;
}

static int BUnpack (Value *retval, int argc __attribute__((unused)), Value *args)
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

static int BTrim (Value *retval, int argc __attribute__((unused)), Value *args)
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

typedef struct PFileComm {
    unsigned magic;
    int lastrc;  /* 0/1/-1  OK/EOF,NOF/Error */
} PFileComm;

#define PFILE_SZIO 0xcabad111

typedef struct PFile {
    PFileComm c;
    SZIO_FILE_ID id;
} PFile;

static int BOpenI (Value *retval, int argc __attribute__((unused)), Value *args)
{
    SZIO_FILE_ID id;
    int rc;
    PFile *pf;

    rc = Szio (SZIO_OPENI | SZIO_MESG | SZIO_OPENFT_TEXT, &id,
	       args[0].u.b.len, args[0].u.b.ptr);

    if (rc==0) {
	pf = emalloc (sizeof (PFile));
	pf->c.magic = PFILE_SZIO;
	pf->c.lastrc = 0;
	pf->id = id;
    } else {
	pf = (PFile *)-1;
    }

    retval->vtype = P_NUMBER;
    retval->u.i = (intptr_t)pf;

    return 0;
}

static int BClose (Value *retval, int argc __attribute__((unused)), Value *args)
{
    PFileComm *pfc;
    int rc;

    pfc = (PFileComm *)args[0].u.i;
    if (pfc==NULL || pfc==(PFileComm *)-1) {
	rc= 0;

    } else if (pfc->magic==PFILE_SZIO) {
	PFile *pf = (PFile *)pfc;
	rc = Szio (SZIO_CLOSE | SZIO_MESG, &pf->id);
	pfc->magic = 0;
	free (pf);

    } else {
	rc= 0;
    }

    retval->vtype = P_NUMBER;
    retval->u.i = rc;

    return 0;
}

static int BGet (Value *retval, int argc __attribute__((unused)), Value *args)
{
    Memory *m;
    PFileComm *pfc;
    size_t reclen;
    char *ptr, *recptr;
    int rc;

    pfc = (PFileComm *)args[0].u.i;
    if (pfc==(PFileComm *)-1) {
	reclen = 0;

    } else if (pfc->magic == PFILE_SZIO) {
	PFile *pf = (PFile *)pfc;

	rc = Szio (SZIO_GET | SZIO_MESG, &pf->id,
		   &reclen, &recptr);
	pfc->lastrc = rc;
	if (rc) reclen= 0;

    } else {
	reclen= 0;
    }

    if (reclen) {
	m = MemGet (reclen);
	ptr = MemAddr (m);
	memcpy (ptr, recptr, reclen);
	retval->vtype = P_STRING;
	retval->u.b.len = reclen;
	retval->u.b.ptr = ptr;
	retval->mem = m;
    } else {
	retval->vtype = P_STRING;
	retval->u.b.len = 0;
	retval->u.b.ptr = NULL;
	retval->mem = NULL;
    }

    return 0;
}

static int BErr (Value *retval, int argc __attribute__((unused)), Value *args)
{
    PFileComm *pf;
    int rc;

    pf = (PFileComm *)args[0].u.i;
    if (pf==(PFileComm *)-1 ||
	(pf->magic != PFILE_SZIO)) rc = -1;
    else rc = pf->lastrc;

    retval->vtype = P_NUMBER;
    retval->u.i = rc;

    return 0;
}

static int BDt (Value *retval, int argc __attribute__((unused)),
    Value *args __attribute__((unused)), int len)
{
    char tmp [64], *p;
    Memory *m;
    time_t tim;
    struct tm tm;

    time (&tim);
    tm = *localtime (&tim);
    strftime (tmp, sizeof (tmp), "%Y%m%d%H%M%S", &tm);

    m= MemGet (len);

    p = MemAddr (m);

    memcpy (p, tmp, len);

    retval->vtype = P_STRING;
    retval->u.b.len = len;
    retval->u.b.ptr = p;
    retval->mem = m;

    return 0;
}

static int BDate   (Value *retval, int argc, Value *args)
{
    return BDt (retval, argc, args, 8);
}

static int BDatime (Value *retval, int argc, Value *args)
{
    return BDt (retval, argc, args, 14);
}
