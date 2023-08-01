/* szio.h */
 
#ifndef SZIO_H
#define SZIO_H

#include <stdint.h>

#include "buffdata.h"
 
typedef intptr_t SZIO_FILE_ID; /* 2nd param for almost all function */
 
extern int Szio (int action,  ...);
 
#define SZIO_OPENI 0      /*  OPEN-INPUT: &fid, int namelen, const char *name */
#define SZIO_OPENO 1      /*  OPEN-OUTPUT                         */
#define SZIO_OPENE 2      /*  OPEN-EXTEND                         */
#define SZIO_CLOSE 16     /*  CLOSE                               */
#define SZIO_CLOSC 17     /*  CLOSE+CUT                           */
#define SZIO_GET   20     /*  GET (IOREG STYLE): LENGTH,ADDRESS   */
			  /*  &fid, size_t *retlen_addr, void **retptr_addr */
#define SZIO_GETV  21     /*  GET (IOREG STYLE): V-RECORD         */
			  /*  &fid, void **retptr_addr 		  */
			  /*  unimplemented			  */
#define SZIO_PUT   30     /*  PUT (WORKA STYLE): LENGTH,ADDRESS   */
#define SZIO_PUTV  31     /*  PUT (WORKA STYLE): V-RECORD         */
#define SZIO_GETN  40     /*  GET FILE NAME: LENGTH,ADDRESS       */
#define SZIO_FNAME 40     /*  GET FILE NAME: LENGTH,ADDRESS       */
			  /*  &fid, size_t *retlen_addr, void **retptr_addr */
#define SZIO_FNAMS 42     /*  opened file names */
#define SZIO_REWIND 50    /*  rewind to the begin of the file */
 
#define SZIO_TIME   8     /*  file modification time */
 
#define SZIO_DUMP  0xE800 /*       ON ERROR: MESSAGE+ABORT        */
#define SZIO_MESG  0xD500 /*       ON ERROR: MESSAGE+RETURN       */
#define SZIO_QUIET 0xD800 /*       ON ERROR: RETURN               */
 
#define SZIO_TZONE  0x030000 /*  TIME: TIMEZONE                    */
#define SZIO_TUNIV  0x000000 /*        UNIVERSAL TIME              */
#define SZIO_TLOCAL 0x010000 /*        LOCAL TIME                  */
#define SZIO_TFORM  0x1C0000 /*  TIME: OUTPUT DATE FORMAT          */
#define SZIO_TCHAR  0x000000 /*        CHAR: YYYY-MM-DD HH:MM      */
#define SZIO_TPCHR  0x040000 /*        CHAR: YYYYMMDDHHMMSS        */
#define SZIO_TBIN   0x080000 /*        BINARY                      */
 
#define SZIO_OPENO_EXIST_MASK       0x030000
#define SZIO_OPENO_EXIST_OVERWRITE  0x000000
#define SZIO_OPENO_EXIST_FAIL       0x010000
#define SZIO_OPENO_EXIST_ONLY       0x020000
 
#define SZIO_OPENFT_BIN   0x00000    /* binary file */
#define SZIO_OPENFT_TEXT  0x40000    /* text file */
 
#define SZIO_RET_EXIST  1      /* file mar letezik */
#define SZIO_RET_NEXIST 2      /* file nem letezik */
 
typedef struct SzioNames {
    BuffData access;    /* DUMMY/FILE/LMS/TIAM */
    BuffData fullname;
    BuffData name;
    BuffData version;
    BuffData type;
    char unused [24];
} SzioNames;
 
#endif
