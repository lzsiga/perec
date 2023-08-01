/* szio.c */

#include <alloca.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "defs.h"
#include "szio.h"
#include "tilog.h"

#define MAXLINE 4000

typedef struct SzioStruct {
    int fclass; /* 0 = std.file; 1 = sysdta/sysout; 2 = dummy */
    FILE *file;
    int	mode; /* 0/1 = input/output */
    char *buff; /* if mode==0 */
    int binary; /* 0/1 = text/binary */
    char *name;
} SzioStruct;

static int DoOpen (int option, va_list ap, int mode,
		   SzioStruct **ret);

int Szio (int option, ...)
{
    va_list ap;
    SzioStruct **pfid, *fid;
    int action, erract, rc;
    struct stat stat;
    void *to;
    size_t *plen, len;
    void **padd, *add;
    size_t bytes;
    char *p;
    int ok;

    va_start (ap, option);
    action = option&255;
    erract = option&0xFF00;
    pfid = va_arg (ap, SzioStruct **);
    switch (action) {
    case SZIO_OPENI:
	rc = DoOpen (option, ap, O_RDONLY, &fid);
	if (rc) return rc; /* Hibakezelés megvolt */
	fid->mode = 0; /* input */
	fid->buff = emalloc (MAXLINE);
	fid->binary = (option & SZIO_OPENFT_TEXT) == SZIO_OPENFT_BIN;
	*pfid = fid;
	return 0;

    case SZIO_OPENO:
	rc = DoOpen (option, ap, O_WRONLY, &fid);
	if (rc) return rc; /* Hibakezelés megvolt */
	fid->mode = 1; /* output */
	fid->buff = NULL;
	fid->binary = (option & SZIO_OPENFT_TEXT) == SZIO_OPENFT_BIN;
	*pfid = fid;
	return 0;

    case SZIO_OPENE:
	rc = DoOpen (option, ap, O_WRONLY | O_APPEND, &fid);
	if (rc) return rc; /* Hibakezelés megvolt */
	fid->mode = 1; /* output */
	fid->buff = NULL;
	fid->binary = (option & SZIO_OPENFT_TEXT) == SZIO_OPENFT_BIN;
	*pfid = fid;
	return 0;
	
    case SZIO_CLOSE: case SZIO_CLOSC:
	fid = *pfid;
	if (fid->fclass==0) { /* csak a normál file-okat kell lezárni */
	    rc = fclose (fid->file);
	} else {
	    rc = 0;
	}
	if (fid->mode==0) free (fid->buff);
	free (fid->name);
	free (fid);
	*pfid = NULL;
	if (rc==0) return 0;
	goto HIBKEZ;

    case SZIO_GET:
	fid = *pfid;
	plen = va_arg (ap, size_t *);
	padd = va_arg (ap, void **);
	if (fid->fclass==2) return 1; /* EOF */

	if (fid->binary) {
	    bytes = fread (fid->buff, 1, MAXLINE, fid->file);
	    ok = bytes>0;
	} else {
	    p = fgets (fid->buff, MAXLINE, fid->file);
	    if (p) {
		ok = 1;
		bytes = strlen (fid->buff);
		if (bytes>0 && fid->buff[bytes-1]=='\n') --bytes;
	    } else {
		bytes = 0;
		ok = 0;
	    }
	}
	if (ok) {
	    *plen = bytes;
	    *padd = fid->buff;
	    return 0;
	} else {
	    *plen = 0;
	    *padd = NULL;
	    if (ferror (fid->file)) goto HIBKEZ;
	    return 1; /* EOF */
	}

    case SZIO_PUT:
	fid = *pfid;
    	len = va_arg (ap, int);
	add = va_arg (ap, void *);

PUTKOZ:	if (fid->fclass==2) return 0; /* eltûnik a semmiben */
	bytes = fwrite (add, 1, len, fid->file);
	ok = bytes==len;
	if (! fid->binary) {
	    fputc ('\n', fid->file);
	}
	if (ok) return 0;
	else return -1;

    case SZIO_PUTV:
	fid = *pfid;
    	va_arg (ap, int); /* no length */
	add = va_arg (ap, void *);
	len = *(unsigned short *)add;
	add = (char *)add + 4;
	len = len-4;
	goto PUTKOZ;

    case SZIO_FNAME:
    	fid = *pfid;
    	plen = va_arg (ap, size_t *);
	padd = va_arg (ap, void **);
	*plen = strlen (fid->name);
	*padd = fid->name;
	return 0;

    case SZIO_FNAMS: {
        SzioNames nam, *pnam;
        SzioStruct *fid;
        size_t len;

    	fid = *pfid;
	len = va_arg (ap, size_t);
	pnam= va_arg (ap, SzioNames *);

	memset (&nam, 0, sizeof (nam));

	if (fid->fclass==0)      nam.access.ptr = "FILE";
	else if (fid->fclass==1) nam.access.ptr = "SYSFILE";
	else                     nam.access.ptr = "DUMMY";
	nam.access.len = strlen (nam.access.ptr);

	nam.fullname.ptr = fid->name;
	nam.fullname.len = strlen (fid->name);

	p = strrchr (fid->name, '/');
	if (p) {
	    nam.name.ptr = p+1;
	    nam.name.len = strlen (nam.name.ptr);
	} else {
	    nam.name = nam.fullname;
	}

	nam.type.ptr = "SAM";
	nam.type.len = 3;

	if (len >= sizeof (SzioNames)) len = sizeof (SzioNames);
	memcpy (pnam, &nam, len);
	return 0; }

    default:
	fprintf (stderr, "*** Szio: Nagy baj van, ismeretlen funcio: %d\n",
	         action);
	exit (12);
    }
    return 0;
HIBKEZ:
    if (erract==SZIO_DUMP || erract==SZIO_MESG) {
        perror ("Error in SZIO");
    }
    if (erract==SZIO_DUMP) {
	exit (12);
    }
    return -1;
}

static int DoFileOpen (const char *fname, int mode,
                       int exopt, FILE **pf)
{
    FILE *f= NULL;
    int rc = -1;
    int h;
    const char *smode;

    if (mode&O_WRONLY) { /* output/extend */
	if (mode&O_APPEND) {
	    mode |= O_CREAT;

	} else if (exopt==SZIO_OPENO_EXIST_OVERWRITE) {
	    mode |= O_CREAT | O_TRUNC;

	} else if (exopt==SZIO_OPENO_EXIST_FAIL) {
	    mode |= O_CREAT | O_EXCL;

	} else if (exopt==SZIO_OPENO_EXIST_ONLY) {
	    if ((mode&O_APPEND)==0) {
		mode |= O_TRUNC;
	    }
	}
	if (mode&O_APPEND) {
	    smode = "a";
	} else {
	    smode = "w";
	}
    } else {
	smode = "r";
    }

    h = open (fname, mode, 0666);
    if (h==EOF) {
	if (errno==ENOENT) {
	    rc = SZIO_RET_NEXIST;

	} else if (errno==EEXIST) {
	    rc = SZIO_RET_EXIST;

	} else {
	    fprintf (stderr, "DEBUG: Szio.open, errno=%d", errno);
	    perror (" ");
	    rc = -1;
	}
	goto RETURN;
    }

    f = fdopen (h, smode);
    if (f==NULL) {
	perror ("Szio.DoFileOpen: fdopen failed");
	close (h);
	rc = -1;
	goto RETURN;
    }
    rc = 0;

RETURN:
    *pf = f;
    return rc;
}

static int DoOpen (int option, va_list ap, int mode,
		   SzioStruct **ret)
{
    FILE *f;
    const char *fname;
    size_t fnamelen;
    SzioStruct *fid= NULL;
    int fclass;
    int rc= -1;
    int exopt, erract;
    char *alloc_fname= NULL;

    fnamelen = va_arg (ap, int);
    fname = va_arg (ap, char *);

    alloc_fname= emalloc (fnamelen+1);
    memcpy (alloc_fname, fname, fnamelen);
    alloc_fname[fnamelen]= '\0';
    fname= alloc_fname;

    if (strcasecmp (fname, "*SYSDTA")==0) {
	fclass = 1;
	f = stdin;
	rc = 0;

    } else if (strcasecmp (fname, "*SYSOUT")==0) {
	fclass = 1;
	f = stdout;
	rc = 0;

    } else if (strcasecmp (fname, "*DUMMY")==0) {
	fclass = 2;
	f = (FILE *)1;
	rc = 0;

    } else {
	fclass = 0;
	exopt = option&SZIO_OPENO_EXIST_MASK;
	rc = DoFileOpen (fname, mode, exopt, &f);

	if (rc==0 && *fname!='/') {
	    char pathbuff [4096], *path, *fnameuj;

	    path = getcwd (pathbuff, sizeof (pathbuff));
	    if (path) {
		fnamelen = strlen (pathbuff) + 1 + strlen(fname);
		fnameuj = alloca (fnamelen + 1);
		sprintf (fnameuj, "%s/%s", pathbuff, fname);
		fname = fnameuj;
	    }
	}
    }

    if (rc) {
	fid= NULL;
	goto HIBKEZ;
    }

    fid = (SzioStruct *)emalloc (sizeof (SzioStruct));
    fid->file = f;
    fid->fclass = fclass;
    fid->name = alloc_fname;
    alloc_fname= NULL;
    rc = 0;
    goto RETURN;

HIBKEZ:
    erract = option&0xFF00;
    if (erract==SZIO_DUMP || erract==SZIO_MESG ||
	erract==(SZIO_DUMP|SZIO_MESG)) {		/* 20150518.LZS */
	if (rc==SZIO_RET_EXIST) {
    	    TiLogF (stderr, 0, "SZIO.Open: *** file \"%.*s\" already exists\n",
		     (int)fnamelen, fname);

	} else if (rc==SZIO_RET_NEXIST) {
    	    TiLogF (stderr, 0, "SZIO.Open: *** file \"%.*s\" does not exist\n",
		     (int)fnamelen, fname);

	} else {
	    TiLogF (stderr, 0, "SZIO.Open: *** error opening file \"%.*s",
		     (int)fnamelen, fname);
	    perror ("\"");
	}
    }
    if (erract==SZIO_DUMP || erract==(SZIO_DUMP|SZIO_MESG)) {
	TiLogF (stderr, 0, "szio: exiting");
	exit (12);
    }
    goto RETURN;

RETURN:
    if (alloc_fname) {
        free (alloc_fname);
        alloc_fname= NULL;
    }
    *ret = fid;
    return rc;
}
