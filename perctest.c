/* perctest.c */

#include <stdio.h>

#include "perec.h"

StaticBD (name1, "elso");
StaticBD (name2, "masodik");

int main (int argc, char *argv[])
{
    Variable *v1, *v2;
    Variable *w1, *w2;

    v1 = VarNew  (&name1);
    v2 = VarNew  (&name2);
    w1 = VarFind (&name1);
    w2 = VarFind (&name2);

    printf ("v1=%p v2=%p w1=%p w2=%p\n"
	    , v1, v2
	    , w1, w2);
    return 0;
}
