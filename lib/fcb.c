/* fcb.c */

#include <inttypes.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "defs.h"
#include "buffdata.h"
#include "tilog.h"

#include "fcb.h"
 
struct FCB {
    char *filename;
    FILE *f;
    int fcbtype;	/* FCB_SAM   / FCB_ISAM   */
    int opmode;		/* FCB_INPUT / FCB_OUTPUT */
    int ioreg;		/* FCB_IOREG / FCB_WORKA  */
    int keyoffs; 	/* 0.. */
    int keylen;
    Buffer lastrec;
    int isbinary;	/* 0: use GetCore/PutCore */
			/* 1: use GetBinCore/PutBinCore */
};

unsigned char fcbXITB;
unsigned short fcbECB;

#define MAXREC 2048

static char *HackFileName (const char *name, char *namebuff, int maxname)
{
    const char *linkname, *filename;
    int fl;

    if (memcmp (name, "link=", 5)!=0 &&
	memcmp (name, "LINK=", 5)!=0) return (char *)name;
    linkname = name+5;

    filename = getenv (linkname);
    if (filename==NULL || *filename=='\0') {
	TiLogF (stderr, 0, "fcb.HackFileName: define env-var '%s'", linkname);
	exit (42);
	return NULL;
    }
    fl = strlen (filename);
    if (fl > maxname+1) {
	TiLogF (stderr, 0, "fcb.c: file-name for link-name %s is too long"
	       , linkname);
	exit (43);
	return NULL;
    }
    memcpy (namebuff, filename, fl);
    namebuff [fl] = '\0';
    return namebuff;
}

FCB *fcbOpen (const char *name, fcb_struct *params)
{
    FILE *f;
    FCB *fcb;
    const char *mode;
    char namebuff [512], *pname;
    int binmode;

    if (!(params->recform==FCB_VAR ||
	 (params->recform==FCB_OLD && params->openmode==FCB_INPUT) ||
	 (params->recform==FCB_BINARY_VAR))) {
OPSUP:
	fprintf (stderr, "*** fcb.fcbOpen: unsupported recform=%d opmode=%d\n"
		 "\tname=%s\n",
		 params->recform, params->openmode, name);
	exit (32);
	return NULL;
    }

    if (params->fcbtype == FCB_SAM);
    else if (params->fcbtype == FCB_ISAM) {
	if (params->keypos < 5 || params->keypos > 256 ||
	    params->keylen < 1 || params->keylen > 256) {
	    fprintf (stderr, "*** fcb.fcbOpen: unsupported ISAM"
		     " keypos=%d, keylen=%d\n"
		    , params->keypos, params->keylen);
	    exit (32);
	    return NULL;
	}
    } else goto OPSUP;

    binmode= (params->recform != FCB_OLD) && (params->recform&FCB_BINARY_REC);

    if (params->openmode == FCB_INPUT) {
	if (binmode) mode= "rb";
	else	     mode= "rt";

    } else if (params->openmode == FCB_OUTPUT) {
	if (binmode) mode= "wb";
	else	     mode= "wt";

    } else {
	goto OPSUP;
    }

    pname = HackFileName (name, namebuff, sizeof (namebuff));

    f = fopen (pname, mode);
    if (!f) {
	int ern= errno;
	TiLogF (stderr, 0, "fcbOpen: *** Error opening file '%s' mode '%s'"
		, name, mode, strerror (ern));
	fcbECB = fcbXITB = errno;
	return NULL;
    }
    fcb = (FCB *)emalloc (sizeof (FCB));
    memset (fcb, 0, sizeof (*fcb));
    
    fcb->filename= estrdup (pname);

    fcb->f = f;
    memset (&fcb->lastrec, 0, sizeof (fcb->lastrec));
    fcb->ioreg = params->ioreg;
    fcb->opmode = params->openmode;

    fcb->fcbtype = params->fcbtype;

    if (fcb->fcbtype == FCB_ISAM) {
	fcb->keyoffs = params->keypos - 5;
	fcb->keylen = params->keylen;
    } else {
	fcb->keyoffs = 0;
	fcb->keylen = 0;
    }

    if (params->recform==FCB_OLD && params->openmode==FCB_INPUT) {
	params->recform= FCB_VAR;
    }

    if (params->recform==FCB_BINARY_FIX) {
	fprintf (stderr, "*** fcb.fcbOpen: binary fix recform is not supported yet");
	exit (32);
	return NULL;

    } else if (params->recform==FCB_BINARY_VAR) {
	fcb->isbinary= 1;

    } else {
	fcb->isbinary= 0;
    }

    return fcb;
}

int fcbClose (FCB *f)
{
    FCB *fcb = (FCB *)f;

    if (fcb->lastrec.ptr) {
	free (fcb->lastrec.ptr);
    }
    fclose (fcb->f);
    if (fcb->filename) free (fcb->filename);
    memset (fcb, 0, sizeof (*fcb));
    free (fcb);
    return 0;
}

static int GetCore (FCB *f, char *holder, int maxlen)
{
    int len;

    len = fgets1 (holder, maxlen, f->f);
    if (len==0) {
	fcbXITB = XITB_EOFADDR;
	len= EOF;
	goto RETURN;

    } else if (len == maxlen-1 && holder [len-1]!='\n') {
	TiLogF (stderr, 0, "fcb.GetCore: *** too long line in '%s' (max=%d), exiting\n",
	    f->filename, maxlen);
	exit (32);
	len= EOF;
	goto RETURN;
    }

    if (holder[len-1]=='\n') holder[--len]= '\0';

RETURN:
    return len;
}

static int GetBinCore (FCB *f, char *holder, int maxlen)
{
    char hdrbuff [4];
    int rdlen, reclen, len;

    rdlen= fread (hdrbuff, 1, 4, f->f);
    if (rdlen==0) {
	fcbXITB = XITB_EOFADDR;
	len= EOF;
	goto RETURN;
    }
    if (rdlen != 4) {
RDB_FILERR:
	fprintf (stderr, "fcb.c: invalid binary file, exiting\n");
	exit (32);
	len= EOF;
	goto RETURN;
    }
    reclen= *(unsigned short *)hdrbuff;

    if (reclen>maxlen) {
	TiLogF (stderr, 0, "fcb.GetBinCore: *** too long line in '%s' (max=%d), exiting\n",
	    f->filename, maxlen);
	exit (32);
	len= EOF;
	goto RETURN;
    }
    if (reclen<4) goto RDB_FILERR;

    if (reclen>4) {
	rdlen= fread (holder, 1, reclen-4, f->f);
	if (rdlen!=reclen-4) goto RDB_FILERR;
    }

    len= reclen-4;

RETURN:
    return len;
}

static char *Return (FCB *fcb, char holder[], int len, char buffer[])
{
    char *ret;

    if (fcb->ioreg == FCB_IOREG) {
	if (fcb->lastrec.ptr==NULL) {
	    fcb->lastrec.ptr = emalloc (4+MAXREC);
	    fcb->lastrec.maxlen = 4+MAXREC;
	}
	ret = fcb->lastrec.ptr;

    } else {
	ret = buffer;
    }

    memcpy (ret+4, holder, len);
    *(unsigned short *)ret = len+4;

    return ret;
}

void *fcbGet (FCB *f, int lock, void *buffer)
{
    FCB *fcb = (FCB *)f;
    int len;
    char holder [MAXREC+1];
    char *ret;

    (void)lock;

    if (fcb->opmode != FCB_INPUT) {
	fprintf (stderr, "fcb.fcbGet: output file!\n");
	exit (34);
    }

    if (f->isbinary) len= GetBinCore (fcb, holder, sizeof (holder));
    else	     len= GetCore (fcb, holder, sizeof (holder));
    if (len==EOF) return NULL;

    ret = Return (fcb, holder, len, buffer);

    return ret;
}

void *fcbGetLn (const FCB *f, const void *buff, size_t *len)
{
    (void)f;

    *len = *(uint16_t *)buff -4;
    return (char *)buff +4;
}

int fcbPut (FCB *f, const void *buffer)
{
    FCB *fcb = (FCB *)f;
    int reclen;
    const void *recptr;
    int wlen;

    if (fcb->opmode != FCB_OUTPUT) {
	fprintf (stderr, "fcb.fcbPut: input file!\n");
	exit (34);
    }

    if (f->isbinary) {
	reclen = *(unsigned short *)buffer;
	recptr = (char *)buffer;

    } else {
	reclen = *(unsigned short *)buffer -4;
	recptr = (char *)buffer + 4;
    }

    wlen = fwrite (recptr, 1, reclen, fcb->f);
    if (wlen != reclen) {
	perror ("fcb.fcbPut: error on write");
    }

    if (!f->isbinary) {
	fputc ('\n', fcb->f);
    }
    return 0;
}

void *fcbGetky (FCB *f, const void *key, int lock, void *buffer)
{
    FCB *fcb = (FCB *)f;
    char holder [MAXREC+1], *ret;
    int len, eof, found;

    (void)lock;

    if (fcb->opmode != FCB_INPUT ||
	fcb->fcbtype != FCB_ISAM) {
	fprintf (stderr, "fcb.fcbGetky: non-isam, or non-input file!\n");
	exit (34);
    }
    rewind (fcb->f);
    for (found= 0, eof= 0; !found && !eof;) {
	len = GetCore (fcb, holder, sizeof (holder));
	if (len==EOF) {
	    eof= 1;
	    continue;
	}
	if (len < fcb->keyoffs + fcb->keylen) {
	    fprintf (stderr, "fcb.fcbGetky: az olvasott rekord túl rövid"
		     " van=%d, min=%d (keyoffs=%d, keylen=%d)\n"
		    , len, fcb->keyoffs + fcb->keylen
		    , fcb->keyoffs, fcb->keylen
		    );
	    exit (35);
    	}
	if (memcmp (holder + fcb->keyoffs, key, fcb->keylen)==0) found= 1;
    }
    if (eof) {
	fcbXITB = XITB_NOFIND;
	return NULL;
    }
    ret = Return (fcb, holder, len, buffer);

    return ret;
}

void fcbCloseAll (void)
{
    TiLogF (stderr, 0, "fcb.c: fcbCloseAll: Not implemented yet");
}

int fcbGetMeta (FCB *f, int select, void *to)
{
    int rc;
    int handle;
    struct stat statbuf;
    int64_t myto;

    if (!to) to= &myto;	/* 'be prepared' say the Scouts */

    if (!f) {
	rc= -2;
	goto RETURN;
    }
    switch (select) {
    case FCBMETA_HANDLE:
    case FCBMETA_MODTIME:
	if (f->f==NULL) {
	    rc= -3;
	    goto RETURN;
	}
	handle= fileno (f->f);
	if (handle==EOF) {
	    rc= -4;
	    goto RETURN;
	}
	if (select==FCBMETA_HANDLE) {
	    *(int *)to= handle;
	    rc= 0;
	    break;

	} else if (select==FCBMETA_MODTIME) {
	    rc= fstat (handle, &statbuf);
	    if (rc) {
		rc= -5;
		goto RETURN;
	    }
	    *(time_t *)to= statbuf.st_mtime;
	    rc= 0;
	    break;

	} else {
	    exit (100);
	}
	break;

    default:
	TiLogF (stderr, 0, "fcb.fcbGetMeta: *** select=0x%x not supported yet"
	    " (or is it a datasize-mismatch?)", (int)select);
	exit (101);
    }
RETURN:
    return rc;
}

int fcbInsrt (FCB *f __attribute__((unused)), const void *buffer __attribute__((unused)))
{
    TiLogF (stderr, 0, "fcbInsert: *** not implemented yet");
    return 0;
}
