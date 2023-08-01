/* defs.c */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__UNIX__)
    #include <unistd.h>
#elif defined (_Windows) || defined (__MSDOS__)
    #include <io.h>
#endif

#include "defs.h"
#include "buffdata.h"
#include "sajatdump.h"
#include "tilog.h"

static char ERRM [] = "*** Out of memory malloc(%lu)\n";
static char ERRC [] = "*** Out of memory calloc(%lu, %lu)\n";
static char ERRR [] = "*** Out of memory realloc(%p,%lu)\n";
static char ERRS [] = "*** Out of memory in strdup(\"%.16s\" len=%lu)";

DLL_PUBLIC (void, efree) (void *ptr)
{
    free (ptr);
}

DLL_PUBLIC (void *, emalloc) (size_t size)
{
    void *ptr;

    ptr = malloc (size);
    if (ptr==NULL && size!=0) {
	TiLogF (stderr, 0, ERRM, (unsigned long)size);
	sajatdump ();
	exit (3001);
    }
    return ptr;
}

DLL_PUBLIC (void *, ecalloc) (size_t size1, size_t size2)
{
    void *ptr;

    ptr = calloc (size1, size2);
    if (ptr==NULL && size1!=0 && size2!=0) {
	TiLogF (stderr, 0, ERRC, (unsigned long)size1, (unsigned long)size2);
	sajatdump ();
	exit (3002);
    }
    return ptr;
}

DLL_PUBLIC (void *, erealloc) (void *p, size_t size)
{
    void *q;

    if (p==NULL && size==0) {	/* hát ez mi ez? */
	q = realloc (p, size);
	return q;

    } else if (size==0) {	/* => free */
	free (p);
	return NULL;

    } else if (p==NULL) {	/* => malloc */
	q = emalloc (size);
	return q;

    } else {			/* valódi realloc */
	q= realloc (p, size);

	if (!q) {
	    TiLogF (stderr, 0, ERRR, p, size);
	    sajatdump ();
	    exit (3003);
	}
	return q;
    }
}

DLL_PUBLIC (char *, estrdup) (const char *s)
{
    size_t netl, brtl;
    char *q;

    if (s && *s) netl= strlen (s);
    else	 netl= 0;
    brtl= netl+1;

    q= malloc (brtl);
    if (!q) {
	TiLogF (stderr, 0, ERRS, s, (unsigned long)brtl);
	sajatdump ();
	exit (3004);
    }
    if (netl) memcpy (q, s, netl);
    q[netl]= '\0';
    return q;
}

DLL_PUBLIC (void *, mf_emalloc) (void *unused, size_t size)
{
    (void)unused;
    return emalloc (size);
}

DLL_PUBLIC (void *, mf_erealloc) (void *unused, void *ptr, size_t size)
{
    (void)unused;
    return erealloc (ptr, size);
}

DLL_PUBLIC (void *, mf_ecalloc) (void *unused, size_t size1, size_t size2)
{
    (void)unused;
    return ecalloc (size1, size2);
}

DLL_PUBLIC (char *, mf_estrdup) (void *unused, const char *s)
{
    (void)unused;
    return estrdup (s);
}

DLL_PUBLIC (void, mf_free) (void *unused, void *ptr)
{
    (void)unused;
    efree (ptr);
}

const memory_fun DefErrMemFun = ErrMemoryFun;

DLL_PUBLIC (char *, estrdup_mf) (const char *s, const memory_fun *mf)
{
    size_t netl, brtl;
    char *q;

    if (s && *s) netl= strlen (s);
    else	 netl= 0;
    brtl= netl+1;

    q= mf->mf_malloc (mf->mf_context, brtl);
    if (!q) {
	TiLogF (stderr, 0, ERRS, s, (unsigned long)brtl);
	sajatdump ();
	exit (3004);
    }
    if (netl) memcpy (q, s, netl);
    q[netl]= '\0';
    return q;
}

DLL_PUBLIC (void *, emalloc_mf) (size_t size, const memory_fun *mf)
{
    void *q;

    q= mf->mf_malloc (mf->mf_context, size);
    if (!q) {
	TiLogF (stderr, 0, ERRM, (unsigned long)size);
	sajatdump ();
	exit (3004);
    }
    return q;
}

DLL_PUBLIC (void, efree_mf) (void *p, const memory_fun *mf)
{
    mf->mf_free (mf->mf_context, p);
}

DLL_PUBLIC (void *, ecalloc_mf) (size_t size, const memory_fun *mf)
{
    void *q;

    q= mf->mf_malloc (mf->mf_context, size);
    if (!q) {
	TiLogF (stderr, 0, ERRM, (unsigned long)size);
	sajatdump ();
	exit (3004);
    }
    if (size) memset (q, 0, size);
    return q;
}

DLL_PUBLIC (void *, erealloc_mf) (void *ptr, size_t newsize, const memory_fun *mf)
{
    void *newptr;

    newptr= mf->mf_realloc(mf->mf_context, ptr, newsize);
    if (newptr==NULL && newsize!=0) {
	TiLogF (stderr, 0, ERRR, ptr, newsize);
	sajatdump ();
	exit (3003);
    }
    return newptr;
}

FILE *efopen (const char *name, const char *mode)
{
    FILE *f;
    int ern;

#if defined (BS2000) || defined (SIE_BS2000)
    extern FILE *ic@fopen (const char *, const char *);

    f = ic@fopen (name, mode);
#else
    f = fopen (name, mode);
#endif

    if (f) return f;
    ern= errno;
    fprintf (stderr, "Error opening file \"%s\" mode \"%s\""
	     " errno=%d: %s\n"
	    , name, mode, ern, strerror (ern));
/*  sajatdump (); */
    exit (3004);
    return NULL;
}

size_t fgets1 (char *to, size_t maxn, FILE *f)
{
    char *p, *plim;
    int c, eol;

    for (eol= 0, p= to, plim= p+maxn-1;
         !eol && p<plim && (c= fgetc (f))!= EOF;) {
	if (c=='\r') continue;
	*p++ = c;
	if (c=='\n') eol= 1;
    }
    *p= '\0';
    return (size_t)(p-to);
}

size_t strntrim (const char *p, size_t maxlen)
{
    size_t len, netlen;

    for (len= netlen= 0; len<maxlen && p[len]!='\0'; ++len) {
	if (p[len] != ' ' &&
	    p[len] != '\t') netlen = len+1;
    }
    return netlen;
}

int fgetslong (DynBuffer *to, FILE *f, int contchar)
{
    char linebuff [512];
    size_t readin, req;
    int leave, rc;
    size_t ll;

    for (leave= 0, rc= 0, readin= 0; ! leave; ) {
        ll = fgets1 (linebuff, sizeof (linebuff), f);
	if (ll==0) {
	    if (ferror (f)) {
		 rc= -1; /* error */

	    } else if (readin) {/* Ha volt már adat, és most EOF jött, */
				/* akkor teszünk oda egy sorvégét */
				/* Note: A másik lehetõség a hibajelzés lenne */ 
		linebuff[0]= '\n';
		ll= 1;
		rc=  0;		/* van adat */

	    } else {
		rc=  1; /* EOF   */
	    }
	    leave= 1;
	    continue;
	}
	if (linebuff[ll-1] == '\n') {
	    if (ll>1 && contchar && linebuff[ll-2]==contchar) {
		ll -= 2;
	    } else {
		leave= 1;
	    }
	    if (ll==0) continue;
	}
    
	readin += ll;
	if (to->maxlen < to->len + ll) {
	    req= to->len + ll - to->maxlen;
	    if (req < to->maxlen) req= to->maxlen;
	    to->maxlen += req;
	    to->ptr = (char *)erealloc (to->ptr, to->maxlen);
	}
	memcpy (to->ptr + to->len, linebuff, ll);
	to->len += ll;
    }
    return rc;
}

#ifndef _GNU_SOURCE
ssize_t getline (char **lineptr, size_t *n, FILE *stream)
{
    ssize_t rlen;

    rlen = getdelim (lineptr, n, '\n', stream);
    return rlen;
}

static void SecSpace (char **lineptr, size_t *n, size_t actsize)
{
    if (actsize==*n) {
        if (*n==0) *n= 256;
        else *n *=2;
        *lineptr = erealloc (*lineptr, *n);
    }
}

ssize_t getdelim (char **lineptr, size_t *n, int delim, FILE *stream)
{
    ssize_t actsize;
    int c, eol;

    for (eol= 0, actsize= 0;
         !eol && (c= fgetc (stream))!= EOF;) {

        if ((size_t)actsize==*n) SecSpace (lineptr, n, actsize);
        (*lineptr) [actsize++] = c;
        if ((char)c==(char)delim) eol= 1;
    }

    if ((size_t)actsize==*n) SecSpace (lineptr, n, actsize);
    (*lineptr) [actsize] = '\0';

    if (actsize) return actsize;
    else return (ssize_t)EOF;
}
#endif
