/* strvaker.h */
 
#ifndef __STRVAKER_H
#define __STRVAKER_H
 
struct strvaker_par {
    char *text;
    int   textlen;
    char *item;
    int   itemlen;
    char *retptr;  /* item's first occurance in text or NULL */
};
 
#ifdef __STDC__
    void strvaker (struct strvaker_par *);
#else
    void strvaker ();
#endif
 
#endif
