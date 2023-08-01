/* tillog.c */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <sys/types.h>

#include "defs.h"

#if defined(__UNIX__)
    #include <unistd.h>

#elif defined (_Windows) || defined (_WIN32)
    #include <windows.h>
    #define getpid() GetCurrentProcessId()
#endif

#include "tilog.h"
#include "tilog_i.h"

/* option: 1=yymmdd.hhmmss
	   2=yyyymmdd.hhmmss
	   3=yyyymmdd.hhmmss.mil
	   4=yyyy-mm-dd hh:mm:ss.mil
*/

DLL_PUBLIC (int, TiLLogF) (int hndto, int option, const char *format, ...)
{
    va_list ap;
    int rlen;

    va_start (ap, format);
    rlen = TiLLogV (hndto, option, format, ap);
    va_end (ap);
    return rlen;
}

static int tillog_lastdateoption= 3;

DLL_PUBLIC (int, TiLogH) (char *to, int option)
{
    struct timeb tb;
    struct tm tm;
static const char y2fmt[] = "%y%m%d.%H%M%S";
static const char y4fmt[] = "%Y%m%d.%H%M%S";
static const char y4fmt_long[] = "%Y-%m-%d %H:%M:%S";
    const char *tf;
    size_t tilen;
    int dateoption= (option&127);

    if (dateoption==0) dateoption= tillog_lastdateoption;
    else               tillog_lastdateoption= dateoption;

    if      (dateoption==1)                  tf = y2fmt;
    else if (dateoption==2 || dateoption==3) tf = y4fmt;
    else if (dateoption==4 || dateoption==5) tf = y4fmt_long;
    else                                     tf = "[date-time]";

#if defined(BS2000) || defined(SIE_BS2000)
    unix_ftime (&tb);
#else
    ftime (&tb);
#endif
#ifdef NO_LOCALTIME_R
    tm = *localtime (&tb.time);
#else
    localtime_r (&tb.time, &tm);
#endif
    tilen = strftime (to, 24, tf, &tm);
    if (dateoption==3 || dateoption==5) {
	tilen += (size_t)sprintf (to+tilen, ".%03d", tb.millitm);
    }
    if (option&TILOG_PID) {
	tilen += (size_t)sprintf (to+tilen, " %lu", (unsigned long)getpid());
    }

    return (int)tilen;
}

int TiLogCore (int hndto, const char *phdrptr, int hdrlen, const char *p, const char *plim)
{
    char tmps2 [4096];
    int rlen, slen;
    const char *q, *pnext;
    ssize_t wlen;
    const char *hdrptr= phdrptr;

    rlen = 0;
    for (; p<plim; p= pnext) {
	q = memchr (p, '\n', plim-p);
	if (q==NULL) pnext= q= plim;
	else	     pnext= q+1;
	slen = sprintf (tmps2, "%*s %.*s\n", hdrlen, hdrptr, (int)(q-p), p);
	rlen += slen;
	wlen= write (hndto, tmps2, slen);
	if (wlen != slen) {
	    fprintf (stderr, "tillog.TiLogCore: wlen=%ld errno=%d\n", (long)wlen, errno);
	}

	hdrptr= ""; /* Csak az elsõnél lesz dátum+idõ */
    }
    return rlen;
}

DLL_PUBLIC (int, TiLLogV) (int hndto, int option, const char *format,
		   va_list ap)
{
    char tbuff [64];
    char tmps [4096];
    size_t len;
    int tilen;
    char *plim;
    int rlen= 0;

    tilen= TiLogH (tbuff, option);

    len = (size_t)vsprintf (tmps, format, ap);
    if (len>0 && tmps[len-1]=='\n')
	 tmps[--len] = '\0';

    plim = tmps+len;

    if (hndto != -1) {
	rlen= TiLogCore (hndto, tbuff, tilen, tmps, plim);
    }
    if (option&TILOG_DEBUGFILE) {
	mode_t old_umask;
	char debugname [256];
	int myhnd;

	sprintf (debugname, "/tmp/debug.%" PRIu64, (uint64_t)getpid ());
	old_umask= umask (0);
	myhnd= open (debugname, O_WRONLY | O_APPEND | O_CREAT, 0666);
	umask (old_umask);
	if (myhnd != -1) {
	    rlen= TiLogCore (myhnd, tbuff, tilen, tmps, plim);
	    close (myhnd);
	}
    }

    return rlen;
}

/* format nélkül, de fejrésszel */
int TiLog (FILE *to, int option, size_t slen, const char *str)
{
    int handle= -1;
    size_t rlen;

    if (slen==0 || str==NULL) return 0;
    if (slen==(size_t)-1) slen= strlen(str);

    if (to) {
	fflush (to);
	handle= fileno (to);
	flockfile (to);
    }
    rlen= TiLLog (handle, option, slen, str);
    if (to) {
	funlockfile (to);
    }
    return rlen;
}

int TiLLog (int hndto, int option, size_t slen, const char *str)
{
    char tbuff [64];
    int tilen;
    const char *p;
    const char *plim;
    int rlen;

    if (slen==0 || str==NULL) return 0;
    if (slen==(size_t)-1) slen= strlen(str);

    p= str;
    plim= str+slen;
    while (plim>p && isspace (plim[-1])) --plim;
    if (plim==p) return 0;

    tilen= TiLogH (tbuff, option);

    rlen= TiLogCore (hndto, tbuff, tilen, p, plim);
    return rlen;
}
