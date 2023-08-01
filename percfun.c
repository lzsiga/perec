/* percfun.h */

#include <stddef.h>
#include <stdlib.h>

#include "defs.h"
#include "perec.h"
struct RealFun;
struct BuffData;
#define BABITED_OBJECT struct RealFun
#define BABIT_KEY      struct BuffData
#include "babit.h"

typedef struct RealFun {
    Function f;
    BabitConnector bc;
} RealFun;

static int VCmpFun (const RealFun *obj1, const RealFun *obj2);
static int VFndFun (const BuffData *key, const RealFun *obj2);

static BABIT_ROOT root = 0;  /* ures fa */

static BabitControlBlock bcb = {
    0,
    offsetof (RealFun, bc),
    VCmpFun,
    VFndFun
};

Function *FunFind (const BuffData *name)
{
    RealFun *r;

    r = BabitFind (&root, name, &bcb);
    return &r->f;
}

Function *FunNew (const BuffData *name)
{
    RealFun *r, *r2;

    r = (RealFun *)ecalloc (sizeof (RealFun), 1);
    r->f.name = *name;
    r2 = BabitInsert (&root, r, &bcb);
    if (r2) { /* already exists */
	free (r);
	return NULL;
    } else {
	return &r->f;
    }
}

static int VCmpFun (const RealFun *obj1, const RealFun *obj2)
{
    int rc;

    rc = BuffDataCmp  (&obj1->f.name, &obj2->f.name);
    return rc;
}

static int VFndFun (const BuffData *key, const RealFun *obj2)
{
    int rc;

    rc = BuffDataCmp  (key, &obj2->f.name);
    return rc;
}
