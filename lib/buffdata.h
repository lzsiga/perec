/* buffdata.h */

#ifndef BUFFDATA_H
#define BUFFDATA_H

#include <stddef.h>

typedef struct BuffData {
    char *ptr;
    size_t len;
} BuffData;

#define EmptyBuffData {NULL, 0}

typedef struct Buffer {
    char *ptr;
    size_t len;
    size_t maxlen;
} Buffer;

#define EmptyBuffer {NULL, 0, 0}
#define EmptyBufferArea(charray) {(charray), 0, sizeof (charray)}

/* Partial check for special uses: */
#define IsBufferSane1(bptr)	\
((bptr)->len   <=(bptr)->maxlen)

/* Correct (total) check: */
#define IsBufferSane(bptr)	\
 (IsBufferSane1(bptr) &&	\
  (((bptr)->maxlen==0) == ((bptr)->ptr==NULL)))

/* RelBuffData, RelBuffer:
   mint BuffData és Buffer, csak pointer helyett offset. */

typedef struct RelBuffData {
    ptrdiff_t  ptr;
    size_t len;
} RelBuffData;

#define EmptyRelBuffData {0, 0}

typedef struct RelBuffer {
    ptrdiff_t  ptr;
    size_t len;
    size_t maxlen;
} RelBuffer;

#define EmptyRelBuffer {0, 0, 0}

#define BD_RelToAbs(rel,base,abs) {\
	(abs)->ptr = (rel)->ptr + (char *)(base);\
	(abs)->len = (rel)->len; }

#define BD_AbsToRel(abs,base,rel) {\
	(rel)->ptr = (abs)->ptr - (char *)(base);\
	(rel)->len = (abs)->len; }

#define Buff_RelToAbs(rel,base,abs) {\
	(abs)->ptr    = (rel)->ptr + (char *)(base);\
	(abs)->len    = (rel)->len;\
	(abs)->maxlen = (rel)->maxlen; }

#define Buff_AbsToRel(abs,base,rel) {\
	(rel)->ptr    = (abs)->ptr - (char *)(base);\
	(rel)->len    = (abs)->len;\
	(rel)->maxlen = (abs)->maxlen; }

/* BasedRelBuffData, BasedRelBuffer:
   RelBuffData és RelBuffer kiegészítve a báziscím
   címével (azért a cím címével, hogy egy esetleges áthelyezés
   (pl realloc) se okozzon gondot */

typedef struct BasedRelBuffData {
    char **base;
    ptrdiff_t  ptr;
    size_t len;
} BasedRelBuffData;

typedef struct BasedRelBuffer {
    char **base;
    ptrdiff_t  ptr;
    size_t len;
    size_t maxlen;
} BasedRelBuffer;

#define BBD_RelToAbs(rel,abs) {\
	(abs)->ptr = (rel)->ptr + *(char **)((rel)->base);\
	(abs)->len = (rel)->len; }

#define BBD_AbsToRel(abs,basep,rel) {\
	(rel)->base= (char **)(basep); \
	(rel)->ptr = (abs)->ptr - *(char **)(basep);\
	(rel)->len = (abs)->len; }

#define BBuff_RelToAbs(rel,abs) {\
	(abs)->ptr = (rel)->ptr + *(char **)((rel)->base);\
	(abs)->len    = (rel)->len;\
	(abs)->maxlen = (rel)->maxlen; }

#define BBuff_AbsToRel(abs,basep,rel) {\
	(rel)->base   = (char **)(basep); \
	(rel)->ptr    = (abs)->ptr - *(char **)(basep);\
	(rel)->len    = (abs)->len;\
	(rel)->maxlen = (abs)->maxlen; }


/* DynBuffer: ugyanaz, mint a Buffer, akkor használhatjuk, ha hangsúlyozni
akarjuk, hogy egy függvény reallocálhatja a paraméterként megkapott Buffert,
ha az kicsi (illetve eleve ptr==NULL, maxlen==0 is lehet) */

typedef struct Buffer DynBuffer;

typedef struct LimBuffer {
    char *ptr;
    size_t len;		/* in best cases len<=maxlen<=limit */
    size_t maxlen;
    size_t limit;
} LimBuffer;

#define EmptyLimBuffer(limit) {NULL, 0, 0, (limit)}

/* Partial check for special uses: */
#define IsLimBufferSane1(bptr)	\
(IsBufferSane1(bptr) &&		\
 ((bptr)->maxlen<=(bptr)->limit))

/* Correct (total) check: */
#define IsLimBufferSane(bptr)	\
(IsBufferSane(bptr) &&		\
 ((bptr)->maxlen<=(bptr)->limit))

typedef struct ConstBuffData {
    const char *ptr;
    size_t len;
} ConstBuffData;

#define StaticBuffData(Name,Sname,Value) \
	static char Sname [] = Value; \
	static BuffData Name = { Sname, sizeof (Sname)-1}

#define StaticBD(Name,Value) StaticBuffData(Name,N##Name,Value)

#define StaticConstBuffData(Name,Sname,Value) \
    static const char Sname [] = Value; \
    static const ConstBuffData Name = { (char *)Sname, sizeof (Sname)-1}

#define StaticConstBD(Name,Value) StaticConstBuffData(Name,N##Name,Value)

/* Egy kifinomultabb megközelítés RelBuffDatára, amely a NULL-pointert a (-1) offsettel
   azonosítja: */

void XBD_RelToAbs (const RelBuffData *from, const void *base, BuffData *to);
void XBD_AbsToRel (const ConstBuffData *from, const void *base, RelBuffData *to);

int BuffDataCmp	 (const ConstBuffData *a, const ConstBuffData *b);
int BuffDataCmpI (const ConstBuffData *a, const ConstBuffData *b);

/* BuffDataCmp[I] returns -1/0/1 for LT/EQ/GT */
/* warning: BuffDataCmpI uses strncasecmp, so does not work well
   if '\0' character found */

int BuffDataEq	(const ConstBuffData *a, const ConstBuffData *b);
int BuffDataEqI (const ConstBuffData *a, const ConstBuffData *b);

/* BuffDataEq[I] returns non-0/0 for EQ/NE */
/* warning: BuffDataEqI uses strncasecmp, so does not work well
   if '\0' character found */

int BuffDataBeg  (const ConstBuffData *data, const ConstBuffData *beg);
int BuffDataBegI (const ConstBuffData *data, const ConstBuffData *beg);

/* BuffDataBeg[I] returns non-0/0 for match/not-match */
/* warning: BuffDataBegI uses strncasecmp, so does not work well
   if '\0' character found */

int BuffDataEnd  (const ConstBuffData *data, const ConstBuffData *end);
int BuffDataEndI (const ConstBuffData *data, const ConstBuffData *end);

/* BuffDataEnd[I] returns non-0/0 for match/not-match */
/* warning: BuffDataEndI uses strncasecmp, so does not work well
   if '\0' character found */

int BuffDataBegX (int popt, const ConstBuffData *blong, const ConstBuffData *bshort, char sep);
#define BBX_OPT_EQ     1 /* teljes egyezés elfogadható-e */
#define BBX_OPT_EQ_OK  0 /* igen, az is jó */
#define BBX_OPT_EQ_NO  1 /* nem, valódi prefix legyen */
#define BBX_OPT_SEP    6 /* kell-e hogy 'sep' character jöjjön a rövidebb után a hosszabban */
#define BBX_OPT_SEP_NO 0 /* nem, 'sep' érdektelen */
#define BBX_OPT_SEP_Y  2 /* igen, 'sep' kell következzen */
#define BBX_OPT_SEP_Y2 4 /* igen, 'sep' kell következzen, és még azután is legyen valami */
#define BBX_OPT_SEP_Y3 6 /* igen, 'sep' kell következzen, és még azután is legyen egy nem 'sep' */
/* pl: blong="libfoo.so.4.6" bshort="libfoo.so" sep='.'
       popt=BBX_OPT_SEP_Y3 -> ok

   pl: blong="libfoo.so.4.." bshort="libfoo.so" sep='.'
       popt=BBX_OPT_SEP_Y2 -> ok
       popt=BBX_OPT_SEP_Y3 -> nem ok

   pl: blong="libfoo.so.4." bshort="libfoo.so" sep='.'
       popt=BBX_OPT_SEP_Y  -> ok
       popt=BBX_OPT_SEP_Y2 -> nem ok
 */

int BuffDataEndX (int popt, const ConstBuffData *blong, const ConstBuffData *bshort, char sep);
/* az opciókat lásd fent */
/* pl: blong="path/.libs" bshort=".libs" sep='/'
       popt=BBX_OPT_SEP_Y3 -> ok

   pl: blong="//.libs" bshort=".libs" sep='/'
       popt=BBX_OPT_SEP_Y2 -> ok
       popt=BBX_OPT_SEP_Y3 -> nem ok

   pl: blong="/.libs" bshort=".libs" sep='/'
       popt=BBX_OPT_SEP_Y  -> ok
       popt=BBX_OPT_SEP_Y2 -> nem ok
 */

int BuffDataCmpX (const ConstBuffData *a, const ConstBuffData *b,
	BuffData *a_comm);
/* an extension: returns the prefix of 'a' which is equal to the
   analoguous part of 'b'
   eg: 'ABC' vs 'ADE' -> 'A';  'ABC' vs 'AB' -> 'AB'
 */

void BuffAppend (DynBuffer *into, const ConstBuffData *from);
/* BuffAppend is the same as BuffInsertAt(into,into->len,from) */

void BuffAppendLP (DynBuffer *into, size_t len, const void *p);
void BuffAppendS  (DynBuffer *into, const char *s);
void BuffAppendChar (DynBuffer *b, char c);

void BuffInsert (DynBuffer *into, const ConstBuffData *from);
/* BuffInsert is the same as BuffInsertAt(into,0,from) */

void BuffInsertAt (DynBuffer *into, size_t tooffs,
		   const ConstBuffData *from);
/* BuffInsertAt call "erealloc", if not enough space in "into" */

void BuffCut (Buffer *from, size_t cutoff, size_t cutlen);

void BuffSecure (Buffer *into, size_t minimum);
/* Secures "minimum" free bytes in "into" Buffer */

void BuffSecX (Buffer *into, size_t minimum);
/* Aggressive version:
    reqsize= minimum - (max-act);
    if (reqsize < max) reqsize= maxsize;
 */

int LimBuffSec (int opt, LimBuffer *into, size_t minimum);
/* Like the previous one, but maxlen<=limit.
   Opt:
    0= don't extend if (len+minimum > limit)
    1= extend even if (len+minimum > limit); maxlen will be limit
   Return value:
    0= ok
    1= len+minimum > limit (see 'opt')
   -1= inconsistent input, eg len>maxlen || (maxlen>0 && ptr==NULL)
 */

void BuffCopy  (Buffer *into, const ConstBuffData *from);
void BuffCopyZ (Buffer *into, const ConstBuffData *from);
/* tesz a végére lezáró nullát is */
void BuffCopyS (Buffer *into, const char *from);
/* tesz a végére lezáró nullát is */

void BuffCutback (Buffer *b, size_t newmaxlen /* >=0 */);
/* Cutback:
    b->maxlen = newmaxlen;
    if (b->len > newmaxlen) b->len= newmaxlen;
    b->ptr = erealloc (b->ptr, b->newmaxlen);
 */

struct memory_fun; /* err_memory_fun értelemben: sikertelen alloc/realloc esetén lépjen ki */
void BuffSec_M    (DynBuffer *into, size_t minimum, const struct memory_fun *memfun);
int  LimBuffSec_M (int opt, LimBuffer *into, size_t minimum, const struct memory_fun *memfun);

void BuffCutback_M    (DynBuffer *b, size_t newmaxlen /* >=0 */, const struct memory_fun *memfun);
void LimBuffCutback_M (LimBuffer *b, size_t newmaxlen /* >=0 */, const struct memory_fun *memfun);

void BuffAppend_M   (DynBuffer *into, const ConstBuffData *from, const struct memory_fun *memfun);
void BuffAppendLP_M (DynBuffer *into, size_t len, const void *p, const struct memory_fun *memfun);
void BuffAppendS_M  (DynBuffer *into, const char *s, const struct memory_fun *memfun);

void BuffAppendChar_mf (DynBuffer *b, char c, const struct memory_fun *memfun);

int LimBuffInsertAt (LimBuffer *into, size_t tooffs, const ConstBuffData *from);
int LimBuffAppend   (LimBuffer *into, const ConstBuffData *from);

void BuffBuild (DynBuffer *into, int n, ...);
/* build a Buffer from n 'ConstBuffData' parts,
   with an optional separator between them:
   
   BuffBuild (into, n,
	ConstBuffData *elem_1,
	char separator, ConstBuffData *elem_2,
	...
	char separator, ConstBuffData *elem_n);
   
   there is no separator before an elem:
   - if separator=='\0'
   - if the elem is empty
   - if this is the first non-empty elem

   - the separator is 'BuffDirSep', and it would generate a '//' sequence eg:
     '/etc/' + '/' + 'hosts.conf' = '/etc//hosts.conf'
     '/etc/' + BuffDirSep + 'hosts.conf' = '/etc/hosts.conf'
*/
#define BuffDirSep 256

void BuffB_PathFile (DynBuffer *into, const ConstBuffData *path, const ConstBuffData *file);
/* Build a full pathname from path and filename parts:
    BuffBuild (into, 2, path, BuffDirSep, file);
*/

void BuffDup (BuffData *into, const ConstBuffData *from);
void BuffDupZ (BuffData *into, const ConstBuffData *from);
/* egy lezáró \0-t is tesz a régi szép idõk emlékére */
void BuffDupS (BuffData *into, const char *from);
/* ez is tesz \0-t a végére */

void BuffRelease (BuffData *mem); /* to release the memory allocated by BuffDup and friends */

void BuffDup_M  (BuffData *into, const ConstBuffData *from, const struct memory_fun *memfun);
void BuffDupZ_M (BuffData *into, const ConstBuffData *from, const struct memory_fun *memfun);
void BuffDupS_M (BuffData *into, const char *from, const struct memory_fun *memfun);
void BuffDupLP_M  (BuffData *into, size_t len, const void *from, const struct memory_fun *memfun);
void BuffDupLPZ_M (BuffData *into, size_t len, const void *from, const struct memory_fun *memfun);

void BuffRelease_M (BuffData *mem, const struct memory_fun *memfun);

void BuffAddTermZero (DynBuffer *b);
/* Adds a terminating zero, without changing 'b->len' */

void BuffAddTermZeroes (DynBuffer *b, unsigned nzero);
/* Adds 1-4 terminating zeroes, without changing 'b->len',
   eg: BuffAddTermZeroes (into, sizeof (wchar_t));
       BuffAddTermZeroes (into, sizeof (uchar32_t));
 */

void BuffAddTermZero_mf (DynBuffer *b, const struct memory_fun *memfun);
void BuffAddTermZeroes_mf (DynBuffer *b, unsigned nzero, const struct memory_fun *memfun);

#endif
