/* naive_parser.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "szio.h"
#include "clex.h"

#include "perec.h"

static LexData ld;

#define Q_A	  (P_FREE + 1)
#define Q_C	  (P_FREE + 2)
#define Q_E	  (P_FREE + 3)
#define Q_X	  (P_FREE + 4)
#define Q_KNUMBER (P_FREE + 5)
#define Q_KSTRING (P_FREE + 6)
#define Q_ELSE	  (P_FREE + 7)
#define Q_BEGIN   (P_FREE + 8)
#define Q_END	  (P_FREE + 9)

static KeyWord kw [] = { /* ABC sorrend ! */
    {"A",      LEX_MIN_TOKEN + Q_A},
    {"AND",    LEX_MIN_TOKEN + P_AND},
    {"BEGIN",  LEX_MIN_TOKEN + Q_BEGIN},
    {"C",      LEX_MIN_TOKEN + Q_C},
    {"E",      LEX_MIN_TOKEN + Q_E},
    {"ELSE",   LEX_MIN_TOKEN + Q_ELSE},
    {"END",    LEX_MIN_TOKEN + Q_END},
    {"FOR",    LEX_MIN_TOKEN + P_FOR},
    {"IF",     LEX_MIN_TOKEN + P_IF},
    {"IN",     LEX_MIN_TOKEN + P_IN},
    {"NOT",    LEX_MIN_TOKEN + P_NOT},
    {"NUMBER", LEX_MIN_TOKEN + Q_KNUMBER},
    {"OR",     LEX_MIN_TOKEN + P_OR},
    {"STRING", LEX_MIN_TOKEN + Q_KSTRING},
    {"WHILE",  LEX_MIN_TOKEN + P_WHILE},
    {"X",      LEX_MIN_TOKEN + Q_X}
};

#define Nkw (sizeof(kw)/sizeof(kw[0]))

#define LINESIZE 4096
#define ITEMSIZE 256

static long lineno= 1;
static char *linebuff= NULL;
static char *userbuff= NULL;

static char *GetLine (unsigned long param)
{
    int rc;
    void *ptr;
    int len;

GET:
    rc = Szio (SZIO_GET | SZIO_DUMP, (SZIO_FILE_ID *)param,
	       &len, &ptr);
    if (rc) return NULL;
    ++lineno;

    if (len>0 && ((char *)ptr)[0]=='#') goto GET;

    if (len > LINESIZE-2) {
	fprintf (stderr, "Too long line #%ld\n", lineno);
	exit (8);
    }
    memcpy (linebuff, ptr, len);
    memcpy (linebuff+len, "\n", 2);

    return linebuff;
}

int PercPars_Naive (SZIO_FILE_ID *id, Codes *c)
{
    linebuff = malloc (LINESIZE);
    userbuff = malloc (ITEMSIZE);
    memset (&ld, 0, sizeof (ld));
    ld.kwords = kw;
    ld.keyno = Nkw;
    ld.rdproc = GetLine;
    ld.llparam = (unsigned long)id;
    ld.userbuff = userbuff;
    ld.flags = LEX_FLAG_STRQ | LEX_FLAG_KEEPNL | LEX_FLAG_UNDER;
    LexInit (&ld, NULL);

    memset (c, 0, sizeof *c);

/*  c->begin = pbegin;
    c->main  = program;
    c->end   = pend; */
    return 0;
}
