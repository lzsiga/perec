/* percmain.c */

#include <dlfcn.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffdata.h"
#include "defs.h"
#include "a_value.h"
#include "szio.h"
#include "tilog.h"

#include "perec.h"

char *progname;

static int SetStrVar (const char *nv);
static int SetIntVar (const char *nv);

StaticBD (Ninput, "INPUT");
StaticBD (Ninplen, "INPLEN");
StaticBD (Nirecno, "IRECNO");

/*** StaticBD (Sinput, "Ez itten az input"); ***/

typedef struct OutDesc {
    SZIO_FILE_ID szid;
    long recno;
} OutDesc;

static struct Vars {
    const char *sprg;
    SZIO_FILE_ID szprg;
    SZIO_FILE_ID szin;
    long irecno;
    int outn;	  /* number of out-files */
    OutDesc *out;
    BuffData irec;
    int debug;
    int stop;
    int quiet;
} var;

static void *GetPlugin (const BuffData *from, BuffData *name);

static void Checkxit (const Code *c)
{
    int rc;

    rc = CodeCheck (c, NULL);
    if (rc) {
	fprintf (stderr, "Exiting\n");
	exit (36);
    }
}

static void Printit (const Code *c, const char *bef, const char *aft)
{
    printf ("%s", bef);
    CodePrint (c);
    printf ("%s", aft);
}

static void IntrHandler (int signo)
{
    TiLLogF (2, 0, "<<< Interrupted signal=%d >>>", signo);
    var.stop = 1;
}

int main (int argc, char *argv[])
{
    Codes p;
    Value value;
    int i, rc, len;
    char *ptr;
    BuffData bind, bname;
    void *pbind;
    Variable *vinput, *vinplen, *virecno;
    Variable *vbind;
    void (*oldh)(int);

    progname = argv[0];

    var.sprg = "*SYSDTA";
    while (--argc>0 && **++argv=='-') {
	switch (argv[0][1]) {
	case 'd': case 'D':
	    var.debug = 1;
	    break;

	case 'b': case 'B':
	    if (strcasecmp (argv[0], "-bind")==0) {
		if (argc==1) goto OPTNOVAL;
		bind.ptr = *++argv;
		bind.len = strlen (bind.ptr);
		--argc;
		pbind = GetPlugin (&bind, &bname);
		vbind = VarNew (&bname);
		vbind->value.vtype = P_NUMBER;
		vbind->value.u.i = (long)pbind;
		break;
	    } else goto INVOPT;

	case 's': case 'S':
	    if (strcasecmp (argv[0], "-strvar")==0) {
		if (argc==1) goto OPTNOVAL;
		rc = SetStrVar (*++argv);
		--argc;
		if (rc) goto OPTBADVAL;
		break;
	    } else goto INVOPT;

	case 'i': case 'I':
	    if (strcasecmp (argv[0], "-intvar")==0) {
		if (argc==1) goto OPTNOVAL;
		rc = SetIntVar (*++argv);
		--argc;
		if (rc) {
OPTBADVAL:	    fprintf (stderr, "Bad value '%s' for option '%s'\n"
			    , argv[0], argv[-1]);
		    return 13;
		}
		break;
	    } else goto INVOPT;

	case 'f': case 'F':
	    if (argc==1) goto OPTNOVAL;
	    var.sprg = *++argv;
	    --argc;
	    break;

	case 'q': case 'Q':
	    if (strcasecmp (argv[0], "-quiet")==0) {
		var.quiet= 1;
		break;
	    } else goto INVOPT;

	default:
INVOPT:     fprintf (stderr, "invalid option '%s'\n", argv[0]);
	    return 12;
OPTNOVAL:   fprintf (stderr, "option '%s' without value\n", argv[0]);
	    return 12;
	}
    }

    if (argc < 1) {
	fprintf (stderr, "usage: perec <input> <output> ...\n");
	return 0;
    }

    vinput = VarNew (&Ninput);
    vinplen = VarNew (&Ninplen);
    virecno = VarNew (&Nirecno);
    vinput->value.vtype = P_STRING;
    vinput->value.mem = NULL;
    vinplen->value.vtype = P_NUMBER;
    virecno->value.vtype = P_NUMBER;

    BltInit ();

    Szio (SZIO_OPENI | SZIO_DUMP | SZIO_OPENFT_TEXT,
	  &var.szprg, (int)strlen (var.sprg), var.sprg);
    PercPars (&var.szprg, &p);
    Szio (SZIO_CLOSE, &var.szprg);

    if (!var.quiet) {
	if (p.begin) Printit (p.begin, "BEGIN {", "} \n");
	if (p.main)  Printit (p.main,  "", "\n");
	if (p.end)   Printit (p.end,   "END {", "} \n");
    }

    if (p.begin) Checkxit (p.begin);
    if (p.main)  Checkxit (p.main);
    if (p.end)	 Checkxit (p.end);

    Szio (SZIO_OPENI | SZIO_DUMP | SZIO_OPENFT_TEXT,
	  &var.szin, strlen (argv[0]), argv[0]);
    var.outn = argc -1;

    if (var.outn) {
	var.out = ecalloc (var.outn, sizeof (OutDesc));
	for (i=0; i<var.outn; ++i) {
	    Szio (SZIO_OPENO | SZIO_DUMP | SZIO_OPENFT_TEXT,
		  &var.out[i].szid,
		  strlen (argv[i+1]), argv[i+1]);
	    var.out[i].recno = 0;
	}
    } else {
	var.out = NULL;
    }

    if (p.begin) {
	rc = CodeCalc (p.begin, &value);
	if (var.debug) {
	    printf ("rc=%d value=", rc);
	    ValuePrint (&value, 1);
	    putchar ('\n');
	    MemStat (0);
	}
	if (rc) {
	    fprintf (stderr, "Exception raised in BEGIN\n");
	    goto VEGE;
	}
    }

    oldh = signal (SIGINT, IntrHandler);
    while (! var.stop &&
	   Szio (SZIO_GET | SZIO_DUMP, &var.szin,
		 &var.irec.len, &var.irec.ptr) ==0) {

	++var.irecno;

	vinput->value.u.b = var.irec;
	vinplen->value.vtype = P_NUMBER;
	vinplen->value.u.i   = var.irec.len;
	virecno->value.u.i   = var.irecno;

	rc = CodeCalc (p.main, &value);
	if (var.debug) {
	    printf ("rc=%d value=", rc);
	    ValuePrint (&value,1);
	    putchar ('\n');
	    MemStat (0);
	}
	MemDropM (&value.mem); /* 20050703.LZS */

	if (rc) {
	    fprintf (stderr, "Exception raised in record #%ld\n",
		     var.irecno);
	    break;
	}
    }
    signal (SIGINT, oldh);
    if (var.stop) {
	    fprintf (stderr, "INTeRrupted after %ld record\n",
		     var.irecno);
    }

    if (p.end) {
	rc = CodeCalc (p.end, &value);
	if (var.debug) {
	    printf ("rc=%d value=", rc);
	    ValuePrint (&value, 1);
	    putchar ('\n');
	    MemStat (0);
	}
	if (rc) {
	    fprintf (stderr, "Exception raised in END\n");
	}
    }

VEGE:
    Szio (SZIO_FNAME | SZIO_DUMP, &var.szin,
	  &len, &ptr);
    if (! var.quiet) {
	printf ("%8ld rec read from    %.*s\n",
		var.irecno, len, ptr);
    }
    Szio (SZIO_CLOSE | SZIO_MESG, &var.szin);

    for (i=0; i<var.outn; ++i) {
	Szio (SZIO_FNAME | SZIO_DUMP, &var.out[i].szid,
	      &len, &ptr);
	if (! var.quiet) {
	    printf ("%8ld rec written into %.*s\n",
		    var.out[i].recno, len, ptr);
	}
	Szio (SZIO_CLOSC | SZIO_DUMP, &var.out[i].szid);
    }
    if (var.outn) free (var.out);

    if (!var.quiet) MemStat (1);

    return 0;
}

int OutPut (int fileno, void *vrec)
{
    if (fileno < 1 || fileno > var.outn) {
	fprintf (stderr, "Invalid output-file number"
		 " %d not in 1..%d\n",
		 fileno, var.outn);
	exit (20);
    }
    Szio (SZIO_PUTV | SZIO_DUMP, &var.out[fileno-1].szid, NULL, vrec);
    ++var.out[fileno-1].recno;
    return 0;
}

static void *GetPlugin (const BuffData *from, BuffData *name)
{
    char sentry[512], slib[512];
    char *colon, *comma;
    void *hlib;
    void *addr;
    BuffData rest, lib, entry;

    rest = *from;
    colon = memchr (rest.ptr, ':', rest.len);
    if (colon==NULL) {
	name->ptr= NULL;
	name->len= 0;
    } else {
	name->ptr= rest.ptr;
	name->len= colon - rest.ptr;
	rest.len -= colon+1 - rest.ptr;
	rest.ptr = colon+1;
    }
    comma = memchr (rest.ptr, ',', rest.len);
    if (comma==NULL) {
	entry.ptr = rest.ptr;
	entry.len = rest.len;
	lib.ptr = "";
	lib.len = 0;
    } else {
	entry.ptr = rest.ptr;
	entry.len = comma - rest.ptr;
	lib.ptr = comma+1;
	lib.len = (rest.ptr + rest.len) - (comma + 1);
    }
    if (name->len==0) *name= entry;

    sprintf (sentry, "%.*s", (int)entry.len, entry.ptr);
    sprintf (slib,   "%.*s", (int)lib.len,   lib.ptr);

    hlib = dlopen (slib, RTLD_NOW);
    if (hlib==NULL) goto PLERROR;
    addr = dlsym (hlib, sentry);
    if (addr==NULL) goto PLERROR;

    printf ("%.*s BOUND at %p symbol %.*s\n",
	    (int)rest.len, rest.ptr, addr,
	    (int)name->len, name->ptr);
    return (void *)addr;

PLERROR:
    fprintf (stderr, "*** PEREC: could not bind module "
	     "'%.*s': %s\n", (int)from->len, from->ptr, dlerror());
    exit (41);
    return NULL;
}

static void SplitAssignment (const char *nv, BuffData *bn, BuffData *bv)
{
    char *p;

    bn->ptr = (void *)nv;
    p = strchr (nv, '=');
    if (p==NULL) {
	bn->len = strlen (p);
	bv->len = 0;
	bv->ptr = (void *)-1;
    } else {
	bn->len = p - nv;
	bv->len = strlen (p+1);
	bv->ptr = p+1;
    }
}

static int SetStrVar (const char *nv)
{
    BuffData bn, bv;
    Variable *v;
    Memory *m;
    char *p;

    SplitAssignment (nv, &bn, &bv);

    v = VarNew (&bn);
    v->value.vtype = P_STRING;

    if (bv.len) {
	m= MemGet (bv.len);
	p = MemAddr (m);
	memcpy (p, bv.ptr, bv.len);
	v->value.u.b.len = bv.len;
	v->value.u.b.ptr = p;
	v->value.mem = m;
    } else {
	v->value.u.b.len = 0;
	v->value.u.b.ptr = NULL;
	v->value.mem = NULL;
    }
    return 0;
}

static int SetIntVar (const char *nv)
{
    BuffData bn, bv;
    Variable *v;
    ValuePar vp;
    int rc;

    SplitAssignment (nv, &bn, &bv);

    if (bv.len == 0) return -1;

    vp.BASE= 10;
    vp.LEN = bv.len;
    vp.ADDR = bv.ptr;

    rc = A_Value (&vp);
    if (rc) return -1;

    v = VarNew (&bn);
    v->value.vtype = P_NUMBER;
    v->value.u.i = vp.BASE;
    v->value.mem = NULL;

    return 0;
}
