/* percfun.h */

#include <stddef.h>
#include <stdlib.h>

#include "defs.h"
#include "perec.h"
#include "buffdata.h"

struct RealFun;
#define BABITED_OBJECT struct RealFun
#define BABIT_KEY      struct BuffData
#include "babit.h"

typedef struct RealFun {
    Function f;
    BabitConnector bc;
} RealFun;

static int VCmpFun (const RealFun *obj1, const RealFun *obj2, BABIT_EXTRAPAR unused);
static int VFndFun (const BuffData *key, const RealFun *obj2, BABIT_EXTRAPAR unused);

static BABIT_ROOT root = 0;  /* ures fa */

static BabitControlBlock bcb = {
    0,
    offsetof (RealFun, bc),
    VCmpFun,
    VFndFun,
    (BABIT_EXTRAPAR)0
};

Function *FunFind (const BuffData *name)
{
    RealFun *r;

    r = BabitFindU (&root, name, &bcb, 0);
    return &r->f;
}

Function *FunNew (const BuffData *name)
{
    RealFun *r, *r2;

    r = (RealFun *)ecalloc (sizeof (RealFun), 1);
    r->f.name = *name;
    r2 = BabitInsertU (&root, r, &bcb, 0);
    if (r2) { /* already exists */
	free (r);
	return NULL;
    } else {
	return &r->f;
    }
}

static int VCmpFun (const RealFun *obj1, const RealFun *obj2, BABIT_EXTRAPAR unused)
{
    int rc;

    (void)unused;
    rc = BuffDataCmp ((ConstBuffData *)&obj1->f.name, (ConstBuffData *)&obj2->f.name);
    return rc;
}

static int VFndFun (const BuffData *key, const RealFun *obj2, BABIT_EXTRAPAR unused)
{
    int rc;

    (void)unused;
    rc = BuffDataCmp ((ConstBuffData *)key, (ConstBuffData *)&obj2->f.name);
    return rc;
}
