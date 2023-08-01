/* defs.h */
 
#ifndef DEFS_H
#define DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#include "buffdata.h"

#if defined(SIE_BS2000) || defined(BS2000)
   #define BS2ONLY(x) x
   #define NOTBS2(x)
#else
   #define BS2ONLY(x)
   #define NOTBS2(x)  x
#endif
 
#if 'A'!=0x41
   #define EBCDICONLY(x) x
#else
   #define EBCDICONLY(x)
#endif

#if 'A'==0x41
   #define ASCIIONLY(x) x
#else
   #define ASCIIONLY(x)
#endif

/* a gyari offsetof egy felesleges warning-ot general clang-ban */
#ifdef offsetof
#undef offsetof
#endif
#define offsetof(T,F) ((size_t)((char *)&(((T *)0)->F)))

#ifndef sizeofrf
    #define sizeofrf(T,F) (sizeof(((T *)0)->F))
#endif
 
#ifndef sizeobj
    #define sizeobj(x) sizeof (x), (x)
#endif

/* sizobj - for an array */
#ifndef sizeobja
    #define sizeobja(x) sizeof (x), (x)
#endif

/* sizeobj - for an object */
#ifndef sizeobjo
    #define sizeobjo(x) sizeof (x), (&(x))
#endif
 
#define NEW(type) ((type *)emalloc(sizeof(type)))
#define NEWS(type,n) ((type *)emalloc(n*sizeof(type)))

/* 20081218: Nem volt jo otlet
 * #ifndef byte
 * #define byte unsigned char
 * #endif
 */

typedef struct va_list_struct {
    va_list ap;	/* platform_dependent: pointer _or_ array */
} va_list_struct;

#ifndef CHARPTR
#define CHARPTR(p) (*(char **)&(p))
#endif
#ifndef UCHARPTR
#define UCHARPTR(p) (*(unsigned char **)&(p))
#endif

typedef int qsort_callback (const void *, const void *);

typedef size_t fwrite_callback (const void *ptr, size_t size, size_t nmemb,
                                intptr_t cbpar);

/* felhasznaloi memoriakezelo fuggvenyek,
   pl amiket az Oracle-OCI is hasznal: */

typedef void *malloc_fun  (void *context, size_t size);
typedef void *realloc_fun (void *context, void *ptr, size_t size);
typedef void free_fun     (void *context, void *ptr);

typedef struct memory_fun {
    void	*mf_context;
    malloc_fun  *mf_malloc;
    realloc_fun *mf_realloc;
    free_fun    *mf_free;
} memory_fun;

#define err_memory_fun memory_fun

/* Megjegyzesek: 

	1. A memory_fun strukturaban nincs 'mf_calloc' fuggveny.
	   Az elet nem habostorta, Pelikan elvtars!

	2. A 'mf_realloc' (eppugy, mint a gyari realloc),
	   kell tudjon foglalni (ptr==NULL),
	   es felszabaditani is (size==0)

	3. Az err_memory_fun ugyanaz, mint a memory_fun, csak hangsulyozza,
	   hogy a hivo nem akar a sikertelen foglalassal bajlodni,
	   hanem elvarja, hogy ilyenkor a mf_malloc/mf_realloc allitsa le
	   a programot (mint az emalloc/erealloc).
 */

/* PHP also has 'emalloc' */
#ifdef emalloc

#undef emalloc
#undef ecalloc
#undef erealloc
#undef estrdup
#undef efree

#define DEFS_EMALLOC_ERRMESSAGE \
Hat kedves baratom, te most egy szerencsetlen utkozes\
aldozata vagy, nevezetesen a PHP-s emalloc es\
a hazibarkacsa emalloc kozott.\
Szerencsere a hiba linkeleskor nem jelentkezik\
(mivel a PHP-s emalloc egy makro az _emalloc-ra),\
de forditaskor igen.\
A megoldast abban latom, hogy a PHP-s emalloc helyett\
az _emalloc-ot hasznald (az az igazi neve);\
a hazibarkacs emalloc helyett\
pedig az mf_emalloc-ot.\
Persze ugyanez vonatkozik a csatolt fuggvenyekre is

#define emalloc  DEFS_EMALLOC_ERRMESSAGE
#define ecalloc  DEFS_EMALLOC_ERRMESSAGE
#define erealloc DEFS_EMALLOC_ERRMESSAGE
#define estrdup  DEFS_EMALLOC_ERRMESSAGE
#define efree    DEFS_EMALLOC_ERRMESSAGE

#else

void *emalloc  (size_t size);
void *ecalloc  (size_t size1, size_t size2);
void *erealloc (void *, size_t size);
char *estrdup  (const char *s);
void  efree    (void *);

#endif

void *mf_emalloc  (void *unused, size_t size);
void *mf_ecalloc  (void *unused, size_t size1, size_t size2);
void *mf_erealloc (void *unused, void *ptr, size_t size);
char *mf_estrdup  (void *unused, const char *s);
void  mf_free     (void *unused, void *ptr);

#define ErrMemoryFun {NULL, mf_emalloc, mf_erealloc, mf_free}

extern const err_memory_fun DefErrMemFun; /* = ErrMemoryFun */

void *emalloc_mf  (size_t size, const memory_fun *mf);
void *ecalloc_mf  (size_t size, const memory_fun *mf);
void *erealloc_mf (void *ptr, size_t newsize, const memory_fun *mf);
char *estrdup_mf  (const char *str, const memory_fun *mf);
void  efree_mf    (void  *p, const memory_fun *mf);

FILE *efopen (const char *name, const char *mode);
size_t fgets1 (char *to, size_t maxn, FILE *f);
/* fgets1 visszaadott ertek:
	0: eof/error
	1..maxn-2: sikeres olvasas
	maxn-1: csonkulas vagy eppen maximalis hosszusagu sor:
	    if (retlen==buffsize-1 && buff [retlen-1]!='\n') {
		... csak a sor elejet olvastuk be,
	    	a tobbit majd egy kovetkezo olvasas ...
	    }
    Megjegyzes: lehet, hogy a fajl vegen nincs sorvege-jel,
    ennek felismerese:
	if (1<retlen && retlen<buffsize-1 && buff [retlen-1]!='\n') {
	    ... nincs sorvege-jel az utolso sor vegen ...
	}
 */

int fgetslong (DynBuffer *to, FILE *f, int contchar);
/* fgetslong megjegyzesek:
   1. Ha nem akarsz folytatosort megengedni, legyen contchar= 0
   2. Meg ha volt is folytatosor, csak az input legvegen lesz '\n',
      a belsejeben nem (es persze a folytatasjelet is kiszuri).
   2. Hivas elott ne felejtsd el: to->len= 0
 */

#ifndef _GNU_SOURCE
/* GNU extensions: getdelim, getline */
extern ssize_t getline  (char **lineptr, size_t *n, FILE *stream);
extern ssize_t getdelim (char **lineptr, size_t *n, int delim, FILE *stream);
#endif

int getfilecontent (int opt, const char *filename, DynBuffer *into);
int putfilecontent (int opt, const char *filename, const ConstBuffData *from);

int getfilecontent_mf (int opt, const char *filename, DynBuffer *into, const err_memory_fun *mf);

#define GFCO_CUTBACK  1 /* beolvasas utan BuffCutBack hivasa */
#define GFCO_VERBOSE  2 /* sikeres muvelet utan uzenet */
#define GFCO_TEXTFILE 4 /* "rt" illetve "wt" modban nyissuk meg */
#define PFCO_APPEND   8 /* "w/wt" helyett "a/at" modban nyissuk meg */
/* getfilecontent filename==NULL (vagy filename=="") eseten a 'stdin'-t olvassa (EOF-ig) */

int getfilecontent_line (int opt, const char *filename, LimBuffer *into, const err_memory_fun *mf);
/* a fajlbol csak az elso/egyetlen sort kerjuk, sorvege jel nelkul (CR vagy LF lehet),
   GFCO_TEXTFILE automatikusan beallitott (azaz "rt" modban olvas),
   a vegere terminalo nullat tesz (kiveve, ha egyaltalan nincs hely)
 */

/*
 * stropt:   visszaadja a hosszt a lezaro szokozok nelkul, \0-t _nem_ figyel
 * strnlen:  visszaadja a hosszt (megkeresi a \0-t), de legfeljebb maxlen-t
 * strntrim: az elozo ketto kombinacioja: \0-t is keres, szokozoket is levag
 *           megj: a string vegen a tabulatorokat is szokoznek veszi
 *	     (vagyis levagja)
 */

extern size_t stropt  (size_t maxlen, const char *p);
extern size_t strnlen (const char *p, size_t maxlen);
extern size_t strntrim (const char *p, size_t maxlen);

struct ConstBuffData;
extern size_t BuffTrim (ConstBuffData *bd, int flags);

#define BTRF_LEFTSP	1 /* vezeto szokozok '   x' --> 'x' */
#define BTRF_LEFTZR	2 /* vezeto nullak   '0000' --> '0' */
#define BTRF_RGHTSP	4 /* lezaro szokozok 'x   ' --> 'x' */
#define BTRF_CSTYLE	8 /* a \0 a string veget jelzi  (ha van) */

#define BTRF_TEXT	13 /* altalanos szoveghez ajanlott  (pl: '  ABC \0___' -> 'ABC' */
#define BTRF_NUM	15 /* szamokhoz ajanlott (pl: '  0012 \0___' -> '12' */

/* 20210916.LZS Az alabbi fuggvenyek eddig a toupper/tolower makrokat hasznaltak,
   ezert mukodesuk esetleges volt.
   Mostantol a 'lat_l2u', 'lat_u2l' translay-tablakat hasznaljak.
 */

char *strupper (char *to, const char *from);
char *strlower (char *to, const char *from);
void  memupper (char *to, const char *from, size_t len);
void  memlower (char *to, const char *from, size_t len);

/*
 * strprefix: ellenorzi, hogy egy string egy prefixszel kezdodik-e
 * vissza: NULL= nem, ptr= a prefix utani resz cime
 *
 * strcaseprefix: ugyanez strcasecmp-vel (kis/nagybetu erzeketlen)
 */

char *strprefix (const char *str, const char *pref);
char *strcaseprefix (const char *str, const char *pref);

#if defined(BS2000) || defined(SIE_BS2000)
/*  In BS2000 you do never know wether timestamps returned by time(2)
 *  are 1950-based or 1970-based.
 *  Moreover, function 'localtime_r' always expects 1970-based
 *  timestamp, unlike function 'localtime'.
 */
    #include "a_gettim.h"
    #define unix_time(tptr)        \
	A_GetTim (GTIM_GETTIME|GTIM_TUNIV|GTIM_TC,(tptr))
    #define unix_ftime(tptr)       \
	A_GetTim (GTIM_GETTIME|GTIM_TUNIV|GTIM_TCMILI,(tptr))
    #define unix_gettimeofday(t,z) \
	A_GetTim (GTIM_GETTIME|GTIM_TUNIV|GTIM_TCMICR,(t))
#else
    #define unix_time(tptr)	   time((tptr))
    #define unix_ftime(tptr)	   ftime((tptr))
    #define unix_gettimeofday(t,z) gettimeofday((t),(z))
#endif

#define timeval_sub(pt2,pt1) \
{\
    (pt2)->tv_sec -= (pt1)->tv_sec;\
    if ((pt2)->tv_usec < (pt1)->tv_usec) {\
	(pt2)->tv_usec += 1000000L;\
	(pt2)->tv_sec--;\
    }\
    (pt2)->tv_usec -= (pt1)->tv_usec;\
}

/* Kulonfele forditok kulonfele szimbolumokat definialnak,
   valasszuk a _Windows-t kozosnek */
#if defined (_WINDOWS) || defined (_WIN32)
#ifndef _Windows
#define _Windows
#endif
#endif

/* Kulonfele forditok kulonfele szimbolumokat definialnak,
   valasszuk a __UNIX__-ot kozosnek */
#if defined (__unix__)
#ifndef __UNIX__
#define __UNIX__
#endif
#endif

#define timeb_diff(pt2,pt1) \
    (1000L*((pt2)->time - (pt1)->time) + \
	  ((pt2)->millitm - (pt1)->millitm))

#if defined (__unix__) || defined (__UNIX__)
    #define DIRSEP '/'
#elif defined (_Windows) || defined (_DOS) || defined (__MSDOS__)
    #define DIRSEP '\\'
#else
    #define DIRSEP '.'
#endif

/* Tried to use _lock_file and _unlock_file but got unresolved externs;
   also (unlike posix) it seems not to supports multiple locks */
#if defined(_Windows)
    #define flockfile(f)   (void)(f)
    #define funlockfile(f) (void)(f)

    #if defined (_WIN64)
	#define gmtime_r(f,t)    _gmtime64_s((t),(f))
	#define localtime_r(f,t) _localtime64_s((t),(f))
    #else
	#define gmtime_r(f,t)    _gmtime32_s((t),(f))
	#define localtime_r(f,t) _localtime32_s((t),(f))
    #endif
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

/* DLL_PUBLIC: olyan szimbolum, amit a DLL exportal */
/* DLL_LOCAL:  olyan szimbolum, amit a DLL nem exportal (azert nem 'static',
	       mert a DLL-en belul tobb modul hasznalja) */
/* 20140506.LZS: Olyasmi ez, amit AIX-en nem tudunk:
    visibility attribute not supported in this configuration; ignored
 */

#ifndef DLL_PUBLIC
/* Oracle Pro*C might not understand any of these */
  #if defined (PREPROCESSOR) || defined (PROC_INVOKED)
    #define DLL_PUBLIC(type,name) type name
    #define DLL_LOCAL(type,name)  type name

/* gcc on AIX can't do this, only gives a warning/error */
  #elif defined (_AIX)
    #define DLL_PUBLIC(type,name) type name
    #define DLL_LOCAL(type,name)  type name

  #elif defined (_Windows)
    #if defined (__GNUC__)
      #define DLL_PUBLIC(type,name) __attribute__ ((dllexport)) type name
      #define DLL_LOCAL(type,name) type name
    #else
      #define DLL_PUBLIC(type,name) type _export name
      #define DLL_LOCAL(type,name) type name
    #endif

  #elif defined (__unix__) || defined (__UNIX__)
    #define DLL_PUBLIC(type,name) __attribute__ ((visibility ("default"))) type name
    #define DLL_LOCAL(type,name)  __attribute__ ((visibility ("hidden"))) type name

  #else /* sigh */
    #define DLL_PUBLIC(type,name) type name
    #define DLL_LOCAL(type,name)  type name

  #endif
#endif

#ifndef B_VarChar
    #define B_VarChar(n) struct { unsigned short len; char arr [(n)]; }
    #define B_UVarChar(n) struct { unsigned short len; unsigned short arr [(n)]; }

    typedef B_VarChar(1)  B_VarChar;
    typedef B_UVarChar(1) B_UVarChar;

    #define B_lenarr(varchar) (varchar)->len, (varchar)->arr
    #define B_arrlen(varchar) (varchar)->arr, (varchar)->len
#endif

#endif
