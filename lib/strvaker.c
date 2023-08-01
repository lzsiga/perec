/* strvaker.c */

#define _GNU_SOURCE
#include <string.h>

#include "strvaker.h"

void strvaker (struct strvaker_par *par)
{
    if (par->itemlen > par->textlen) {
	par->retptr = NULL;
	return;
    }
    if (par->itemlen == 0) {
	par->retptr = par->text;
	return;
    }

#if defined(NO_MEMMEM)
    /* ganyoljunk lelkesen! */
    {
        int bt = par->text[par->textlen];
        int bi = par->item[par->itemlen];
        if (bt) par->text[par->textlen] = 0;
        if (bi) par->item[par->itemlen] = 0;

        par->retptr = strstr (par->text, par->item);

        if (bt) par->text[par->textlen] = bt;
        if (bi) par->item[par->itemlen] = bi;
    }
#else
    par->retptr= memmem (par->text, par->textlen, par->item, par->itemlen);
#endif
}
