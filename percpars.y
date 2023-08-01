%{  /* percpars.y */

#include <alloca.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "buffdata.h"
#include "szio.h"
#include "clex.h"

#include "perec.h"

static int FromHex (const BuffData *from);

#define NB(t,l,r) NewBinary((t),(l).obj,(r).obj)
#define NU(t,l)   NewUnary ((t),(l).obj)
#define NN(v)	  NewNumber((v).value)
#define NS(v,opt) NewString(&(v).b, (opt)); \
		  if ((v).b.len) free ((v).b.ptr);
#define NSV(v)	  NewNumber(FromHex (&(v).b)); \
		  if ((v).b.len) free ((v).b.ptr);
#define NV(v)	  NewVar (&(v).b); \
		  if ((v).b.len) free ((v).b.ptr);
#define CALL(f,l) NewFunc(&(f).b, (l).obj); \
		  if ((f).b.len) free ((f).b.ptr);
#define SECT(f,l) NewSect((f).obj, (l).obj);
#define LET(op,v,l)  NewLet(op, &(v).b, (l).obj); \
		     if ((v).b.len) free ((v).b.ptr);
#define NIF2(c,t)   NewIf((c).obj, (t).obj, NULL);
#define NIF3(c,t,e) NewIf((c).obj, (t).obj, (e).obj)
#define NIFC(c,t,e) NewIfC((c).obj, (t).obj, (e).obj)
#define NWHILE(c,t) NewWhile((c).obj, (t).obj)
#define NFOR(b,c,a,t) NewFor((b).obj, (c).obj, (a).obj, (t).obj);

typedef union stacktype {
    Code *obj;
    int     value;
    BuffData b;
} stacktype;

#define YYSTYPE stacktype

#define YYDEBUG 0
#define YYSTACK_USE_ALLOCA 1

static void yyerror (const char *s);
static int  yylex (void);

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

static char *linebuff = NULL;
static char *userbuff = NULL;

static Code *program = NULL;
static Code *pbegin  = NULL;
static Code *pend    = NULL;
static int lasttype = -1;

%}

%expect 1

%token	IDENT NUMBER STRING NOT AND OR IN
%token	IF ELSE WHILE FOR
%token	A E C X
%token	KNUMBER KSTRING
%token	BEGIN END
%token	EQ NE LE GE
%token	LL LP LM PP MM
%token	DD
%%

root: o_dekls
      o_begin
      o_prog
      o_end;

o_begin:		{ pbegin = NULL; }
    | BEGIN block	{ pbegin = $2.obj; };
o_prog: 		{ program = NULL; }
    | prog		{ program = $1.obj; };
o_end:			{ pend	 = NULL; }
    | END   block	{ pend	 = $2.obj; };

block: '{' code '}'	{ $$.obj = $2.obj; };

code: o_dekls prog	{ $$.obj = $2.obj; };

o_dekls:
    | dekls;
dekls:	dekl ';'
      | dekl ';' dekls
      ;
dekl:	type vars
      ;
vars:	var		{ CreateVar (&$1.b, lasttype); }
      | var ',' vars	{ CreateVar (&$1.b, lasttype); }
      ;
type:	KNUMBER 	{ lasttype = P_NUMBER; }
      | KSTRING 	{ lasttype = P_STRING; }
      ;
prog:	expr		{ $$.obj = $1.obj; }
      | stmt		{ $$.obj = $1.obj; }
      | stmt prog	{ $$.obj = NB (P_SEQ, $1, $2); }
      ;
stmt:	expr ';'	{ $$.obj = $1.obj; }
      | block		{ $$.obj = $1.obj; }
      | if		{ $$.obj = $1.obj; }
      | while		{ $$.obj = $1.obj; }
      | for		{ $$.obj = $1.obj; }
      | ';'		{ $$.obj = NULL; }
      ;
if:	IF '(' expr ')' stmt	       { $$.obj = NIF2($3, $5);	    }
      | IF '(' expr ')' stmt ELSE stmt { $$.obj = NIF3($3, $5, $7); }
      ;
while: WHILE '(' expr ')' stmt	       { $$.obj = NWHILE($3, $5); }
      ;
for:   FOR '(' expr0 ';' expr ';' expr0 ')' stmt
				{ $$.obj = NFOR($3, $5, $7, $9); }
      ;
expr0:				{ $$.obj = NULL; }
      | expr			{ $$.obj = $1.obj; }
      ;
expr:	econd			{ $$.obj = $1.obj; }
      | var LL expr		{ $$.obj = LET (P_LET, $1, $3); }
      | var LP expr		{ $$.obj = LET (P_INC, $1, $3); }
      | var LM expr		{ $$.obj = LET (P_DEC, $1, $3); }
      ;
econd:	eor			{ $$.obj = $1.obj;	    }
      | eor '?' econd ':' econd { $$.obj = NIFC($1, $3, $5); }
      ;
eor:	eand		  { $$.obj = $1.obj; }
      | eor  OR  eand	  { $$.obj = NB (P_OR, $1, $3); }
      ;
eand:	enot		  { $$.obj = $1.obj; }
      | eand AND enot	  { $$.obj = NB (P_AND, $1, $3); }
      ;
enot:	erel		  { $$.obj = $1.obj; }
      | NOT erel	  { $$.obj = NU (P_NOT, $2); }
      | '!' erel	  { $$.obj = NU (P_NOT, $2); }
      ;
erel:	eadd		  { $$.obj = $1.obj; }
      | eadd EQ  eadd	  { $$.obj = NB (P_EQ,	$1, $3); }
      | eadd NE  eadd	  { $$.obj = NB (P_NE,	$1, $3); }
      | eadd '<' eadd	  { $$.obj = NB (P_LT,	$1, $3); }
      | eadd '>' eadd	  { $$.obj = NB (P_GT,	$1, $3); }
      | eadd LE  eadd	  { $$.obj = NB (P_LE,	$1, $3); }
      | eadd GE  eadd	  { $$.obj = NB (P_GE,	$1, $3); }
      | eadd IN  inset	  { $$.obj = NB (P_IN,	$1, $3); }
      | eadd NOT IN inset { $$.obj = NB (P_NIN, $1, $4); }
      ;
eadd:	emul		  { $$.obj = $1.obj; }
      | eadd '+' emul	  { $$.obj = NB (P_ADD, $1, $3); }
      | eadd '-' emul	  { $$.obj = NB (P_SUB, $1, $3); }
      ;
emul:	esign		  { $$.obj = $1.obj; }
      | emul '*' esign	  { $$.obj = NB (P_MUL, $1, $3); }
      | emul '/' esign	  { $$.obj = NB (P_DIV, $1, $3); }
      | emul '%' esign	  { $$.obj = NB (P_MOD, $1, $3); }
      ;
esign:	eval		{ $$.obj = $1.obj; }
      | '-' eval	{ $$.obj = NU (P_NEG, $2); }
      ;
eval:	NUMBER			  { $$.obj = NN ($1); }
      | '#' STRING		  { $$.obj = NSV ($2); }
      | incvar			  { $$.obj = $1.obj; }
      | string			  { $$.obj = $1.obj; }
      | '[' expr ',' expr ']'	  { $$.obj = SECT ($2, $4); }
      | '(' expr ')'		  { $$.obj = $2.obj; }
      | fname '(' list ')'	  { $$.obj = CALL ($1, $3);}
      ;
incvar: var		{ $$.obj = NV ($1); }
      | PP var
	 { $$.obj = NewLet (P_INC, &$2.b, NewNumber (1));
	   if (($2).b.len) free (($2).b.ptr); }
      | var PP
	 { $$.obj = NewLet (P_INCPOST, &$1.b, NewNumber (1));
	   if (($1).b.len) free (($1).b.ptr); }
      | MM var
	 { $$.obj = NewLet (P_DEC, &$2.b, NewNumber (1));
	   if (($2).b.len) free (($2).b.ptr); }
      | var MM
	 { $$.obj = NewLet (P_DECPOST, &$1.b, NewNumber (1));
	   if (($1).b.len) free (($1).b.ptr); }
      ;
fname:	var		{ $$.b	 = $1.b; }
      ;
string: STRING		{ $$.obj = NS ($1, 0); }
      | C STRING	{ $$.obj = NS ($2, 0); }
      | X STRING	{ $$.obj = NS ($2, 1); }
      | A STRING	{ $$.obj = NS ($2, 2); }
      | E STRING	{ $$.obj = NS ($2, 3); }
      ;
var:	IDENT		{ $$.b = $1.b; }
      | C		{ $$.b = $1.b; }
      | X		{ $$.b = $1.b; }
      | A		{ $$.b = $1.b; }
      | E		{ $$.b = $1.b; }
      ;
inset:	'(' inlist ')'	  { $$.obj = $2.obj; }
      ;
inlist: 		{ $$.obj = NULL;   /* empty list */}
      | neinlist	{ $$.obj = $1.obj; /* not empty list */}
      ;
neinlist: ninelem	       { $$.obj = $1.obj; }
	| ninelem ',' neinlist { $$.obj = NB (P_LIS, $1, $3); }
      ;
ninelem: expr		   { $$.obj = $1.obj; }
       | expr DD expr	   { $$.obj = NB (P_INT, $1, $3); }
      ;
list:			   { $$.obj = NULL;   /* empty list */}
      | nelist		   { $$.obj = $1.obj; /* not empty list */}
      ;
nelist: nelem		   { $$.obj = $1.obj; }
      | nelem ',' nelist   { $$.obj = NB (P_LIS, $1, $3); }
      ;
nelem: expr		   { $$.obj = $1.obj; }
      ;

%%

extern char	*progname;
static long	lineno = 1;

static void yyerror(const char *s);
static void warning(const char *s,const char *t);

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

static struct {
    int set;
    int c;
    YYSTYPE yylval;
} stack = {
    0
};

int PercPars (SZIO_FILE_ID *id, Codes *c)
{
/*  yydebug = 0;  */

    memset (&stack, 0, sizeof (stack));

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
    yyparse();

    c->begin = pbegin;
    c->main  = program;
    c->end   = pend;
    return 0;
}

StaticBD (NVA, "A");
StaticBD (NVC, "C");
StaticBD (NVE, "E");
StaticBD (NVX, "X");

static int lexsub (void);

static int lexmerge (int cold, int n, ...)
{
    va_list ap;
    int i, c, c2, r2;
    YYSTYPE yylvalold;
    int found;

    yylvalold = yylval;

    c = lexsub ();
    va_start (ap, n);
    for (i=0, found=0; !found && i<n; ++i) {
	c2 = va_arg (ap, int);
	r2 = va_arg (ap, int);
	found = (c == c2);
    }
    va_end (ap);
    if (found) return r2;

    stack.set = 1;
    stack.c = c;
    stack.yylval = yylval;

    yylval = yylvalold;
    return cold;
}

static int yylex (void)
{
    int c;

    if (stack.set==0) {
	c = lexsub ();
    } else {
	stack.set = 0;
	c = stack.c;
	yylval = stack.yylval;
    }

    if (c=='!')      return lexmerge ('!', 1, '=', NE);
    else if (c=='=') return lexmerge ('=', 1, '=', EQ);
    else if (c=='<') return lexmerge ('<', 2, '>', NE, '=', LE);
    else if (c=='>') return lexmerge ('>', 1, '=', GE);
    else if (c=='+') return lexmerge ('+', 2, '=', LP, '+', PP);
    else if (c=='-') return lexmerge ('-', 2, '=', LM, '+', MM);
    else if (c==':') return lexmerge (':', 1, '=', LL);
    else if (c=='.') return lexmerge ('.', 1, '.', DD);
    else if (c=='|') return lexmerge ('|', 1, '|', OR);
    else if (c=='&') return lexmerge ('&', 1, '&', AND);
    else return c;
}

static int lexsub (void)
{
    int c;

    while ((c = LexGet (&ld)) == '\n');
    if (c==LEX_EOF) return 0;
    if (c==LEX_INT) {
	sscanf (userbuff, "%d", &yylval.value);
	return NUMBER;
    }
    if (c == LEX_STR) {
	if (ld.itemlen) {
	    yylval.b.ptr = emalloc (ld.itemlen);
	    memcpy (yylval.b.ptr, ld.itemadd, ld.itemlen);
	} else {
	    yylval.b.ptr = (void *)-1;
	}
	yylval.b.len = ld.itemlen;
	return STRING;
    }
    if (c == LEX_IDENT) {
	if (ld.itemlen) {
	    yylval.b.ptr = emalloc (ld.itemlen);
	    memcpy (yylval.b.ptr, ld.itemadd, ld.itemlen);
	} else {
	    yylval.b.ptr = (void *)-1;
	}
	yylval.b.len = ld.itemlen;
	return IDENT;
    }
    if (c == LEX_MIN_TOKEN + P_AND) return AND;
    if (c == LEX_MIN_TOKEN + P_NOT) return NOT;
    if (c == LEX_MIN_TOKEN + P_OR)  return OR;
    if (c == LEX_MIN_TOKEN + P_IN)  return IN;
    if (c == LEX_MIN_TOKEN + Q_A) { yylval.b = NVA; return A; }
    if (c == LEX_MIN_TOKEN + Q_C) { yylval.b = NVC; return C; }
    if (c == LEX_MIN_TOKEN + Q_E) { yylval.b = NVE; return E; }
    if (c == LEX_MIN_TOKEN + Q_X) { yylval.b = NVX; return X; }
    if (c == LEX_MIN_TOKEN + Q_KNUMBER) return KNUMBER;
    if (c == LEX_MIN_TOKEN + Q_KSTRING) return KSTRING;
    if (c == LEX_MIN_TOKEN + P_IF)	return IF;
    if (c == LEX_MIN_TOKEN + Q_ELSE)	return ELSE;
    if (c == LEX_MIN_TOKEN + P_WHILE)	return WHILE;
    if (c == LEX_MIN_TOKEN + P_FOR)	return FOR;
    if (c == LEX_MIN_TOKEN + Q_BEGIN)	return BEGIN;
    if (c == LEX_MIN_TOKEN + Q_END)	return END;
    return c;
}

static void yyerror (const char *s)
{
    warning(s, (char *)0);
}

static void warning (const char *s,const char *t)
{
    fprintf(stderr, "%s: %s", progname, s);
    if (t)
	fprintf(stderr, "%s", t);
    fprintf (stderr, " near line %ld\n", lineno);
}

static int FromHex (const BuffData *from)
{
    int len;
    char *ptr;
    int value, c;

    value = 0;
    len = from->len;
    ptr = from->ptr;
    if (len<=0) goto ERR;
    while (--len >= 0) {
	value <<= 4;
	c= *ptr++;
	if (c >= '0' && c<= '9') value += c-'0';
	else if (c >= 'A' && c<= 'F') value += c-'A'+10;
	else if (c >= 'a' && c<= 'f') value += c-'a'+10;
	else goto ERR;
    }
    return value;
ERR:
    fprintf (stderr, "Invalid hex value '%.*s'\n",
	     (int)from->len, from->ptr);
    return -1;
}
