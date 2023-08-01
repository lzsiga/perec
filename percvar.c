/* percvar.c */

#include <stddef.h>
#include <stdlib.h>

#include "defs.h"
#include "buffdata.h"
#include "perec.h"

struct RealVar;
#define BABITED_OBJECT struct RealVar
#define BABIT_KEY      struct BuffData
#include "babit.h"

typedef struct RealVar {
    Variable v;
    BabitConnector bc;
} RealVar;

static int VCmpFun (const RealVar *obj1, const RealVar *obj2, BABIT_EXTRAPAR unused);
static int VFndFun (const BuffData *key, const RealVar *obj2, BABIT_EXTRAPAR unused);

static BABIT_ROOT root = 0;  /* ures fa */

static BabitControlBlock bcb = {
    0,
    offsetof (RealVar, bc),
    VCmpFun,
    VFndFun,
    (BABIT_EXTRAPAR)0
};

Variable *VarFind (const BuffData *name)
{
    RealVar *r;

    r = BabitFindU (&root, name, &bcb, 0);
    return &r->v;
}

Variable *VarNew (const BuffData *name)
{
    RealVar *r, *r2;

    r = (RealVar *)ecalloc (sizeof (RealVar), 1);
    r->v.name = *name;
    r2 = BabitInsertU (&root, r, &bcb, 0);
    if (r2) { /* already exists */
	free (r);
	return NULL;
    } else {
	return &r->v;
    }
}

static int VCmpFun (const RealVar *obj1, const RealVar *obj2, BABIT_EXTRAPAR unused)
{
    int rc;

    (void)unused;
    rc = BuffDataCmp ((ConstBuffData *)&obj1->v.name, (ConstBuffData *)&obj2->v.name);
    return rc;
}

static int VFndFun (const BuffData *key, const RealVar *obj2, BABIT_EXTRAPAR unused)
{
    int rc;

    (void)unused;
    rc = BuffDataCmp ((ConstBuffData *)key, (ConstBuffData *)&obj2->v.name);
    return rc;
}
