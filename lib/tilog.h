/* tilog.h */

#ifndef TILOG_H
#define TILOG_H

#include <stdarg.h>
#include <stdio.h>

/* option: 1=yymmdd.hhmmss
	   2=yyyymmdd.hhmmss
	   3=yyyymmdd.hhmmss.mil
	   4=yyyy-mm-dd hh:mm:ss
	   5=yyyy-mm-dd hh:mm:ss.mil
     (6-127  foglalt k�l�nf�le tov�bbi fejr�sz-form�tumoknak)
	   0=previous (best for subroutins)
	 128=f�jlba is �rjon: /tmp/debug.<PID> -- mode=append
	     (ha emellett to==NULL vagy hndto==-1,
	      akkor �rtelemszer�en _csak_ f�jlba �r)
*/

#define TILOG_PID	  8	/* PID is legyen benne */
#define TILOG_HDRFORMAT 127
#define TILOG_DEBUGFILE 128	/* a filen�v: /tmp/debug.<PID> */

int TiLogF (FILE *to, int option, const char *format, ...);
int TiLogV (FILE *to, int option, const char *format,
                   va_list ap);

/* The following versions use A_FORMAT */

int TiLogAF (FILE *to, int option, const char *format, ...);
int TiLogAV (FILE *to, int option, const char *format,
                   va_list ap);

/* The following versions use write(2) instead of fwrite(3) */

int TiLLogF (int hndto, int option, const char *format, ...);
int TiLLogV (int hndto, int option, const char *format,
		    va_list ap);

int TiLLogAF (int hndto, int option, const char *format, ...);
int TiLLogAV (int hndto, int option, const char *format,
		     va_list ap);

/* format n�lk�l, de fejr�sszel (slen==-1: use strlen) */
int TiLog  (FILE *to,  int option, size_t slen, const char *str);
int TiLLog (int hndto, int option, size_t slen, const char *str);

/* Just create the header (returns the length): */

int TiLogH (char *to, int option);

#endif
