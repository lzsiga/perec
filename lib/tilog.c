/* tilog.c */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "defs.h"

#ifdef _Windows
#include <io.h>
#endif

#include "tilog.h"

/* option: 1=yymmdd.hhmmss
	   2=yyyymmdd.hhmmss
*/

DLL_PUBLIC (int, TiLogF) (FILE *to, int option, const char *format, ...)
{
    va_list ap;
    int rlen;
    int handle= -1;

    va_start (ap, format);
    if (to) {
	fflush (to);
	handle= fileno (to);
	flockfile (to);
    }
    rlen = TiLLogV (handle, option, format, ap);
    va_end (ap);
    if (to) {
	funlockfile (to);
    }
    return rlen;
}

DLL_PUBLIC (int, TiLogV) (FILE *to, int option, const char *format,
                   va_list ap)
{
    int rlen;
    int handle= -1;

    if (to) {
	fflush (to);
	handle= fileno (to);
	flockfile (to);
    }
    rlen = TiLLogV (handle, option, format, ap);
    if (to) {
	funlockfile (to);
    }
    return rlen;
}
