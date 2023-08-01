/* percvar.c */

#include <stddef.h>
#include <stdlib.h>

#include "defs.h"
#include "perec.h"
struct RealVar;
struct BuffData;
#define BABITED_OBJECT struct RealVar
#define BABIT_KEY      struct BuffData
#include "babit.h"

typedef struct RealVar {
    Variable v;
    BabitConnector bc;
} RealVar;

static int VCmpFun (const RealVar *obj1, const RealVar *obj2);
static int VFndFun (const BuffData *key, const RealVar *obj2);

static BABIT_ROOT root = 0;  /* ures fa */

static BabitControlBlock bcb = {
    0,
    offsetof (RealVar, bc),
    VCmpFun,
    VFndFun
};

Variable *VarFind (const BuffData *name)
{
    RealVar *r;

    r = BabitFind (&root, name, &bcb);
    return &r->v;
}

Variable *VarNew (const BuffData *name)
{
    RealVar *r, *r2;

    r = (RealVar *)ecalloc (sizeof (RealVar), 1);
    r->v.name = *name;
    r2 = BabitInsert (&root, r, &bcb);
    if (r2) { /* already exists */
	free (r);
	return NULL;
    } else {
	return &r->v;
    }
}

static int VCmpFun (const RealVar *obj1, const RealVar *obj2)
{
    int rc;

    rc = BuffDataCmp  (&obj1->v.name, &obj2->v.name);
    return rc;
}

static int VFndFun (const BuffData *key, const RealVar *obj2)
{
    int rc;

    rc = BuffDataCmp  (key, &obj2->v.name);
    return rc;
}
