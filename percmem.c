/* percmem.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"

#include "perec.h"

static int MemDb = 0, MemSum = 0;

Memory *MemGet (int size)
{
    Memory *m;

    if (size < 0) size = 0;
    m = (Memory *)emalloc (sizeof (Memory) + size);
    m->refcount = 1;
    m->size = size;
    MemSum += size;
    MemDb ++;
    return m;
}

void MemDrop (Memory **mp)
{
    Memory *m;

    m = *mp;
    if (m==NULL) return;
    if (m->refcount<=0) {
	fprintf (stderr, "MemDrop: m=%p, refcount=%d size=%d\n",
		 (void *)m, m->refcount, m->size);
	exit (44);

    } else if (m->refcount==1) {
	m->refcount= 0;
	MemSum -= m->size;
	MemDb  --;
	memset (MemAddr(m), '*', m->size);
	free (m);
	*mp = NULL;

    } else --m->refcount;
}

void MemStat (int opt)
{
    if (opt==0 || MemSum || MemDb) {
	printf ("MemStat: %d bytes in %d object\n",
		MemSum, MemDb);
    }
}
