/* perccalc.c */

#include <stdio.h>
#include <stdlib.h>

#include "defs.h"

#include "perec.h"

static int ValueCmp (const Value *lval, const Value *rval)
{
    int cmp;

    if (lval->vtype == P_NUMBER) {
	cmp = lval->u.i > rval->u.i ? 1 :
	      lval->u.i < rval->u.i ? -1 : 0;
    } else {
	if (lval->u.b.ptr == rval->u.b.ptr) {
	    cmp = lval->u.b.len > rval->u.b.len ? 1 :
		  lval->u.b.len < rval->u.b.len ? -1 : 0;
	} else {
	    cmp = BuffDataCmp ((ConstBuffData *)&lval->u.b, (ConstBuffData *)&rval->u.b);
	}
    }
    return cmp;
}

static int ValueNotNull (const Value *value)
{
    int ret;

    if (value->vtype == P_NUMBER) {
	ret = value->u.i != 0;	       /* nulla */
    } else {
	ret = value->u.b.len != 0;     /* ures string */
    }
    return ret;
}

static void SetNullValue (int type, Value *value)
{
    value->vtype = type;
    if (type == P_NUMBER) {
	value->u.i = 0; 	/* nulla */
    } else {
	value->u.b.ptr = "";
	value->u.b.len = 0;	/* ures string */
    }
}

static int InCore (const Value *pval, const Code *p, Value *value)
{
    const Code *first, *secnd;
    int rc, found;
    Value lval, rval;

    rc = found = 0;
    while (rc==0 && found==0 && p!=NULL) {
	if (p->type == P_LIS) {
	    first = p->left;
	    secnd = p->right;
	} else {
	    first = p;
	    secnd = NULL;
	}
	if (first->type == P_INT) { /* intervallum */
	    lval.mem = rval.mem = NULL;
	    if ((rc = CodeCalc (first->left, &lval))==0  &&
		ValueCmp (pval, &lval) >= 0		 &&
		(rc = CodeCalc (first->right, &rval))==0 &&
		ValueCmp (pval, &rval) <= 0) {
		found = 1;
	    }
	    MemDropM (&lval.mem);
	    MemDropM (&rval.mem);
	} else {		    /* egyszeru kifejezes */
	    rval.mem = NULL;
	    if ((rc = CodeCalc (first, &rval)) == 0 &&
		ValueCmp(pval, &rval)==0) {
		found = 1;
	    }
	    MemDropM (&rval.mem);
	}
	p = secnd;
    }
    if (value) {
	value->vtype = P_NUMBER;
	value->u.i = found;
    }
    return rc;
}

static int CalcArgs (const Code *p, int *pargc, Value **pargs)
{
    const Code *first, *secnd;
    Value *args;
    int i, argc;
    int rc;

    rc = 0;
    args = NULL;
    argc = 0;

    while (rc==0 && p!=NULL) {
	if (p->type == P_LIS) {
	    first = p->left;
	    secnd = p->right;
	} else {
	    first = p;
	    secnd = NULL;
	}
	++argc;
	if (args==NULL) args = (Value *)emalloc (sizeof (Value));
	else args = (Value *)erealloc (args, argc * sizeof (Value));

	rc = CodeCalc (first, &args[argc-1]);

/* Megszüntetve: 20050703. LZS.
 * Beillesztve:  20050301. LZS.
 *
 *	  if (rc==0) {
 *	      if (args[argc-1].mem) MemUse (&args[argc-1].mem);
 *	  }
 */
	p = secnd;
    }
    if (rc) {
	if (args) {
	    for (i=0; i<argc; ++i) {
		MemDropM (&args[i].mem);
	    }
	    free (args);
	}
	argc = 0;
	args = NULL;
    }

    *pargc = argc;
    *pargs = args;
    return rc;
}

int CodeCalc  (const Code *p, Value *into)
{
    int rc;
    Value retval, lval, rval;
    Value *args, *v;
    int i, argc;

ELEJE:
    rc = 0;
    retval.vtype = P_NUMBER;
    retval.u.i = 0;
    retval.mem = NULL;
    lval.mem = NULL;
    rval.mem = NULL;

    if (p == NULL) {
	rc = -1;
	goto VEGE;
    }

    switch (p->type) {
    case P_CONST:
	retval = p->value;
	goto VEGE;

    case P_VAR:
	retval = p->var->value;
	MemUse (&retval.mem);
	goto VEGE;

    case P_FUNC:
	args = NULL;
	rc = CalcArgs (p->left, &argc, &args);
	if (rc==0) {
	    retval.mem= NULL;
	    rc = p->fun->bltin (&retval, argc, args);
	}
	if (args) {
	    for (i=0; i<argc; ++i) {
		MemDropM (&args[i].mem);
	    }
	    free (args);
	}
	goto VEGE2;

    case P_AND: case P_OR:
    case P_NEG: case P_NOT:
    case P_IN:	case P_NIN:
    case P_LET:
    case P_INC: case P_DEC:
    case P_INCPOST:
    case P_DECPOST:
	rc = CodeCalc (p->left, &lval);
	break;

    case P_EQ:	case P_NE:  case P_LT:	case P_GT:
    case P_LE:	case P_GE:
    case P_ADD: case P_SUB:
    case P_MUL: case P_DIV: case P_MOD:
	rc = CodeCalc (p->left, &lval);
	if (rc==0) rc = CodeCalc (p->right, &rval);
	break;

    case P_SEQ:
	rc = CodeCalc (p->left, &lval);
	if (rc==0) {
	    MemDropM (&lval.mem);
	    p= p->right;
	    goto ELEJE;
	}
	break;

    case P_IF: case P_IFC:
	rc = CodeCalc (p->left, &lval);
	if (rc==0) {
	    i = ValueNotNull (&lval);
	    MemDropM (&lval.mem);
	    if (i)  p= p->right;
	    else    p= p->third;
	    if (p)  goto ELEJE;
	    else    goto VEGE;
	}
	break;

    case P_WHILE:
	while ((rc= CodeCalc (p->left, &lval))==0 &&
	       ValueNotNull (&lval)) {
	    MemDropM (&lval.mem);
	    if ((rc= CodeCalc (p->right, NULL)) != 0) break;
	}
	break;

    case P_FOR:
	if (p->left) {
	    rc= CodeCalc (p->left, NULL);
	    if (rc) break;
	}
	while ((rc= CodeCalc (p->right, &lval))==0 &&
	       ValueNotNull (&lval)) {
	    MemDropM (&lval.mem);
	    if (p->fourth != NULL &&
		(rc= CodeCalc (p->fourth, NULL)) != 0) break;
	    if (p->third != NULL &&
		(rc= CodeCalc (p->third, NULL)) != 0) break;
	}
	break;

    default:
	fprintf (stderr, "CodeCalc: invalid type %d\n", p->type);
	rc = -1;
    }

    if (rc!=0) goto VEGE;

    switch (p->type) {
    case P_LET:
	v= &p->var->value;
	MemDropM (&v->mem);  /* regi erteket 'eldobjuk' */
	retval = *v = lval;
    /*	lval.mem = NULL; */
	MemUse (&v->mem);    /* uj erteket 'megfogjuk' */
	MemUse (&retval.mem); /* mégegyszer megfogjuk */
	break;

    case P_INC:
	if (p->var->value.vtype == P_STRING) {
	    Value args[2];

	    v = &p->var->value;
	    args[0] = *v;
	    args[1] = lval;
	    rc = BStrCat (&retval, 2,  args);
	    if (rc==0) {
		MemDropM (&v->mem);  /* regi erteket 'eldobjuk' */
		*v = retval;
		MemUse (&v->mem);    /* uj erteket 'megfogjuk' */
	    }
	    break;
	} else {
	    p->var->value.u.i += lval.u.i;
	    retval.u.i = p->var->value.u.i;
	}
	break;

    case P_DEC:
	p->var->value.u.i -= lval.u.i;
	retval.u.i = p->var->value.u.i;
	break;

    case P_INCPOST:
	retval.u.i = p->var->value.u.i;
	p->var->value.u.i += lval.u.i;
	break;

    case P_DECPOST:
	retval.u.i = p->var->value.u.i;
	p->var->value.u.i -= lval.u.i;
	break;

    case P_AND:
	if (! ValueNotNull (&lval)) {
	    retval = lval;
	} else {
	    rc = CodeCalc (p->right, &rval);
	    if (rc==0) {
		if (ValueNotNull (&rval)) retval = lval;
		else SetNullValue (lval.vtype, &retval);
	    }
	}
	break;

    case P_OR:
	if (ValueNotNull (&lval)) {
	    retval = lval;
	} else {
	    rc = CodeCalc (p->right, &rval);
	    if (rc==0) retval = rval;
	}
	break;

    case P_EQ:	retval.u.i = ValueCmp (&lval, &rval) == 0; break;
    case P_NE:	retval.u.i = ValueCmp (&lval, &rval) != 0; break;
    case P_LT:	retval.u.i = ValueCmp (&lval, &rval) <	0; break;
    case P_GT:	retval.u.i = ValueCmp (&lval, &rval) >	0; break;
    case P_LE:	retval.u.i = ValueCmp (&lval, &rval) <= 0; break;
    case P_GE:	retval.u.i = ValueCmp (&lval, &rval) >= 0; break;
    case P_SUB: retval.u.i = lval.u.i - rval.u.i; break;
    case P_MUL: retval.u.i = lval.u.i * rval.u.i; break;
    case P_DIV: retval.u.i = lval.u.i / rval.u.i; break;
    case P_MOD: retval.u.i = lval.u.i % rval.u.i; break;
    case P_NEG: retval.u.i = - lval.u.i;	  break;
    case P_NOT: retval.u.i = ! ValueNotNull (&lval); break;

    case P_ADD:
	if (lval.vtype == P_NUMBER) {
	    retval.u.i = lval.u.i + rval.u.i;
	    break;
	} else {
	    Value args[2];

	    args[0] = lval;
	    args[1] = rval;
	    rc = BStrCat (&retval, 2,  args);
	    break;
	}

    case P_IN:
	rc = InCore (&lval, p->right, &retval);
	break;

    case P_NIN:
	rc = InCore (&lval, p->right, &retval);
	if (rc==0) retval.u.i = !retval.u.i;
	break;

    default:
	break;
    }
VEGE:
    MemDropM (&lval.mem);
    MemDropM (&rval.mem);
VEGE2:
    if (into) {
	*into = retval;
    } else {
	MemDropM (&retval.mem);
    }
    return rc;
}
