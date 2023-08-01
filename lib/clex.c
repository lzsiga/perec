/* clex.c */
 
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include "clex.h"
 
#define LEX_EOS (-2)
#define NO_UNGET 256
 
typedef int cmpfun (const void *, const void *);
 
void LexInit (LexData *l, char *s)
{
    l->ptr =  (s != NULL) ? (unsigned char *)s : 
              (l->rdproc) ? (unsigned char *)(l->rdproc)(l->llparam) : 
              NULL;
    l->unget = l->ptr ? NO_UNGET : LEX_EOF;
}
 
static int isletter (int c, unsigned flags)
{
    static char MagicChars [] = "@~#`+$&!\"%_-:.";
    static char HunChars [] = "äáéíóöõúüûÄÁÉÍÓÖÕÚÜÛ";
    char *p;
 
    if (c == 0) return 0;
    if (isalpha (c)) return 1;
 
    if ((flags & LEX_FLAG_LATIN) &&
        strchr (HunChars, c)) return 1;
 
    p = strchr (MagicChars, c);
    if (p == NULL) return 0;
    return (flags >> (p - MagicChars))&1;
}
 
static int cmp (const char *key, const KeyWord *table_elem)
{
    return strcmp (key, table_elem->Word);
}
 
int LexGet (LexData *l)
{
    unsigned flags;
    int c, ret;
    unsigned char *p, *q, *trident;
    KeyWord *kw;
 
    if (l->unget == LEX_EOF) return l->item = LEX_EOF;
    flags = l->flags;
    trident = (unsigned char *)l->trident;
    p = l->ptr;
    if (l->unget == LEX_EOS) {
GETL:   if (l->rdproc == NULL ||
            (p = (unsigned char *)(l->rdproc)(l->llparam)) == NULL)
            return l->item = l->unget = LEX_EOF;
        l->unget = NO_UNGET;
    }
    c = *p++;
 
    while (isspace (c)) {
        if (c=='\n' && (flags & LEX_FLAG_KEEPNL) != 0) {
            goto CHAR;
        }
        c = *p++;
    }
    l->itemadd = (char *)(p-1);
 
    if (c == '\0') {
        if (flags & LEX_FLAG_RETNL) {
            l->unget = LEX_EOS;
            l->itemlen = 1;
            l->itemadd = "\n";
            return l->item = '\n';
        } else {
            goto GETL;
        }
    }
    q = (unsigned char *)l->userbuff;
    if (isdigit (c)) {
        do {
            *q++ = c;
            c = *p++;
        } while (isdigit (c));
        *q = '\0';
        ret = LEX_INT;
        goto ITEMDEC;
    }
    if (isletter (c, flags)) {
        do {
            if (trident) c = trident[c];
            *q++ = c;
            c = *p++;
        } while (isdigit (c) || isletter (c, flags));
        *q = '\0';
        kw = (KeyWord *)bsearch (l->userbuff, l->kwords, l->keyno,
            sizeof (KeyWord), (cmpfun *)cmp);
        ret = kw ? kw->Key : LEX_IDENT;
        goto ITEMDEC;
    }
    if ((c=='\'' && (flags & LEX_FLAG_STRQ)) ||
        (c=='\"' && (flags & LEX_FLAG_STRQQ))) {
        l->strchar = c;
STRLOOP:
        while (*p != c && *p != 0) {
            *q++ = *p;
            ++p;
        }
        if (*p == c) {
            ++p;
            if (*p == c && (flags & LEX_FLAG_STRDUP)) {
                *q++ = *p++;
                goto STRLOOP;
            }
        }
        ret = LEX_STR;
        goto ITEM;
    }
CHAR:
    l->ptr = p;
    l->itemlen = 1;
    return l->item = c;
ITEMDEC:
    --p;
ITEM:
    l->ptr = p;
    l->itemadd = l->userbuff;
    l->itemlen = q - (unsigned char *)l->userbuff;
    return l->item = ret;
}
