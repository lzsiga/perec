/* fcb.h */

#ifndef fcb_h
#define fcb_h

#include <stddef.h>

extern unsigned char  fcbXITB; /*  byte-os hibakod */
extern unsigned short fcbECB;  /*  dms hibakod */

typedef struct FCB FCB;

typedef struct fcb_struct {
    unsigned short fcbtype;
    unsigned short openmode;
    unsigned short blksize;  /*  (STD,n)  --->	blksize = FCB_STD + n */
    unsigned short recform;
    unsigned short recsize;
    unsigned short keypos;
    unsigned short keylen;
    unsigned short dupeky;
    unsigned short sharupd;
    unsigned short ioreg;
} fcb_struct;

#define FCB_OLD       0xFFFF

/* fcbtype */

#define FCB_SAM       0x00
#define FCB_ISAM      0x40
#define FCB_PAM       0xC0
#define FCB_BTAM      0x80

/* openmode */

#define FCB_INPUT     0x01
#define FCB_REVERSE   0x02
#define FCB_OUTPUT    0x04
#define FCB_EXTEND    0x08
#define FCB_UPDATE    0x10
#define FCB_INOUT     0x20
#define FCB_OUTIN     0x40

/* blksize */

#define FCB_STD       0x8000

/* recform */

#define FCB_FIX       0x04
#define FCB_VAR       0x02

#define FCB_BINARY_REC 0x80	/* bináris fájl, sorvégék nélkül */
#define FCB_BINARY_FIX 0x84	/* bináris fájl, sorvégék nélkül, fix rekordok */
#define FCB_BINARY_VAR 0x82	/* bináris fájl, sorvégék nélkül, változó rekordok, */
				/* négybájtos rekord-fejrész: 2 hossz (platformfüggõ), */
				/* 2 unused*/

/* dupeky */

#define FCB_DUPEKY    0x20
#define FCB_NODUPEKY  0x00

#define FCB_WEAKSHARUPD  0x08
#define FCB_SHARUPD	 0x04
#define FCB_NOSHARUPD	 0x00

#define FCB_IOREG     0x01
#define FCB_WORKA     0x00

#define SETL_BOTTOM   (void *)0
#define SETL_END      (void *)1

#define ELIM_LAST     (void *)0

#define FCB_LOCK      1
#define FCB_NOLOCK    0

#define FCB_UNLOCK_THIS 0
#define FCB_UNLOCK_ANY	1

#define fcbUnlock fcbUnlo

/*
      A fuggvenyek visszaadott erteke:

	  fcbOpen  : FCB cime vagy NULL
	  fcbClose : 0 vagy DMS hibakod

	  fcbGet, fcbGetr, fcbGetky :
		     rekord cime vagy NULL

	  fcbPut, fcbPutx, fcbStore, fcbInsrt,
	  fcbSetl, fcbElim :
		     0 vagy DMS hibakod

*/

/* fcbXITB ertekei: */

#define XITB_NOERROR 0x00
#define XITB_EOFADDR 0x40
#define XITB_ERRADDR 0x44
#define XITB_DUPEKY  0x54
#define XITB_NOFIND  0x58

/* Kiegeszitesek:

   fcbGetLn: a rekordhosszat kiolvassa a rekordbol/FCB-bol,
	     ezaltal a fix es valtozo file-ok egysegesen kezelhetok
	     a vissza-adott pointer az elso adatbyte-ra mutat

   fcbCloseAll: lezarja az osszes fcbOpen -nel megnyitott file-t
	     nem azonos az 'CLOSE ALL' lal !

   fcbUnlock: elengedi az ISAM-lockot (esetleg egy masik file-on is)

*/

#define fcbCloseAll FCB_CLA

FCB   *fcbOpen	(const char *name, fcb_struct *params);
int    fcbClose (FCB *f);
void   fcbCloseAll (void);

void  *fcbGet	 (FCB *f, int lock, void *buffer);
void  *fcbGetr	 (FCB *f, int lock, void *buffer);
void  *fcbGetky  (FCB *f, const void *key, int lock, void *buffer);

void  *fcbGetLn  (const FCB *f, const void *buff, size_t *len);

int    fcbPut	 (FCB *f, const void *buffer);
int    fcbPutx	 (FCB *f, const void *buffer);
int    fcbStore  (FCB *f, const void *buffer);
int    fcbInsrt  (FCB *f, const void *buffer);

int    fcbSetl	 (FCB *f, const void *key);
int    fcbElim	 (FCB *f, const void *key);

int    fcbUnlock (FCB *f, int option);

int    fcbGetMeta (FCB *f, int select, void *to);
/* select: see FCBMETA_***
   to:     the return value (type depends on 'select')

 return: 0=OK, other=error
 */

#define FCBMETA_HANDLE  (0x00 + (sizeof (int)))		/* output: int */
#define FCBMETA_MODTIME (0x10 + (sizeof (time_t)))	/* output: time_t */

#endif
