/* clex.h */
 
#ifndef _clex_h
#define _clex_h
 
/* Return values of LexGet */
 
#define LEX_EOF   (-1)
#define LEX_EOL   '\n'
#define LEX_INT    400
#define LEX_STR    500
#define LEX_IDENT  600
 
#define LEX_MIN_TOKEN  1000
#define LEX_MAX_TOKEN 29999
 
/* Flags of parsing */
 
#define LEX_FLAG_MONKEY     1  /* @ is letter */
#define LEX_FLAG_OVER       2  /* ~ is letter */
#define LEX_FLAG_HASH       4  /* # is letter */
#define LEX_FLAG_REVAP      8  /* ` is letter */
#define LEX_FLAG_PLUS      16  /* + is letter */
#define LEX_FLAG_DOLLAR    32  /* $ is letter */
#define LEX_FLAG_ET        64  /* & is letter */
#define LEX_FLAG_EXCL     128  /* ! is letter */
#define LEX_FLAG_QUOTE    256  /* " is letter */
#define LEX_FLAG_PERC     512  /* % is letter */
#define LEX_FLAG_UNDER   1024  /* _ is letter */
#define LEX_FLAG_MINUS   2048  /* - is letter */
#define LEX_FLAG_COLON   4096  /* : is letter */
#define LEX_FLAG_POINT   8192  /* . is letter */
#define LEX_FLAG_LATIN  0x100000 /* hungarian characters of latin2 */

#define LEX_FLAG_HUNBM   1023  /* Hungarian letters are letters */
#define LEX_FLAG_RETNL  32768  /* ret end of input as '\n' */
#define LEX_FLAG_KEEPNL 16384  /* ret '\n' as '\n' */
#define LEX_FLAG_STRQ   0x10000  /* handle '\'' as string separator */
#define LEX_FLAG_STRQQ  0x20000  /* handle '\"' as string separator */
#define LEX_FLAG_STRDUP 0x80000  /* '' -> ' or "" -> " */
 
#define WORD unsigned long
 
#ifdef __STDC__
typedef char *LexLine (WORD);       /* at EOF returns NULL */
#else
typedef char *LexLine (/* WORD */); /* at EOF returns NULL */
#endif
 
typedef struct KeyWord {
    char *Word;
    int   Key;   /* LEX_MIN_TOKEN .. LEX_MAX_TOKEN */
} KeyWord;
 
typedef struct LexData {
/* input */
    LexLine *rdproc;   /* The 'Get-Line' function or NULL */
    WORD     llparam;  /* Parameter for the 'Get-Line' function */
    char    *trident;  /* Translate-table for identifiers or NULL */
    KeyWord *kwords;   /* alphabetically sorted table of keywords */
    int      keyno;    /* number of keywords */
    unsigned flags;
    char    *userbuff; /* Buffer for 'integer', 'ident', ... */
/* output */
    int      item;     /* Result of parsing */
    char    *itemadd;  /* Address of item in the source string */
    int      itemlen;  /* Length of the item in the source string */
    char     strchar;  /* ' or " */
/* private */
    unsigned char    *ptr;
    int      unget;
} LexData;
 
#ifdef __STDC__
   void LexInit   (LexData *l, char *data);
   int  LexGet    (LexData *l);
#else
   void LexInit   ();
   int  LexGet    ();
#endif
 
#endif
