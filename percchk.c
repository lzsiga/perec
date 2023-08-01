/* percchk.c */

#include <stdio.h>
#include <stdlib.h>

#include "defs.h"

#include "perec.h"

static int FunCheck (Function *fun, const Code *parms, int *ptype)
{
    int rc;
    int argc, argmin, argmax, atypno;
    const Code *check;
    VType partype, argtype, lastargtype;

    argmin = fun->minargc;
    argmax = fun->maxargc;
    atypno = fun->atypno;
    if (atypno) lastargtype = fun->atype [atypno - 1];
    else	lastargtype = P_ANY;
    rc = 0;
    for (argc= 0; rc==0 && argc<argmax && parms!=NULL; ++argc) {
	if (parms->type==P_LIS) {
	    check = parms->left;
	    parms = parms->right;
	} else {
	    check = parms;
	    parms = NULL;
	}
	rc = CodeCheck (check, &partype);
	if (rc==0) {
	    if (argc < atypno) argtype = fun->atype [argc];
	    else	       argtype = lastargtype;
	    if (partype != argtype &&
		argtype != P_ANY) {
		fprintf (stderr, "wrong type in arg #%d"
			 " function %.*s\n",
			 argc+1, (int)fun->name.len, fun->name.ptr);
		rc = -1;
	    }
	}
    }
    if (rc==0) {
	if (argc>=argmax && parms!=NULL) {
	    fprintf (stderr, "too many args in function %.*s"
		     " (max %d)\n",
		     (int)fun->name.len, fun->name.ptr, argmax);
	    rc = -1;
	} else if (argc<argmin && parms==NULL) {
	    fprintf (stderr, "too few args in function %.*s"
		     " (min %d)\n",
		     (int)fun->name.len, fun->name.ptr, argmin);
	    rc = -1;
	}
    }
    if (ptype) *ptype= fun->rtype;
    return rc;
}

int CodeCheck (const Code *p, int *ptype)
{
    int rc;
    int type, ltype, rtype, ttype;

    rc=0;
    type= P_VOID;

ELEJE:
    if (p==NULL) {
	rc=-1;
	type= -1;
	goto VEGE;
    }

    switch (p->type) {
	case P_CONST:
	   type = p->value.vtype;
	   break;

	case P_VAR:
	   type = p->var->value.vtype;
	   break;

	case P_SEQ:
	   rc = CodeCheck (p->left, &ltype);
	   if (rc==0) { p= p->right; goto ELEJE; }
	   break;

	case P_FUNC:
	   rc = FunCheck (p->fun, p->left, &type);
	   break;

	case P_LET:
	   rc = CodeCheck (p->left, &ltype);
	   if (rc) break;
	   if (p->var->value.vtype != ltype) goto TYPERR;
	   type = ltype;
	   break;

	case P_INC: case P_INCPOST:
	case P_DEC: case P_DECPOST:
	   rc = CodeCheck (p->left, &ltype);
	   if (rc) break;
/*
 * 20050706.LZS
 * stringek esetében is lehet S1 += S2
 * alakot használni. (++,--,-= továbbra is csak számokra!)
 */
	   if ((ltype != P_NUMBER && p->type != P_INC) ||
	       p->var->value.vtype != ltype) goto TYPERR;
	   type = ltype;
	   break;

	case P_IF:
	   rc = CodeCheck (p->left, &ltype);
	   if (rc) break;
	   rc = CodeCheck (p->right, &rtype);
	   if (rc) break;
	   if (p->third) {
	       rc = CodeCheck (p->third, &ttype);
	       if (rc) break;
	   }
	   type = P_VOID;
	   break;

	case P_IFC:
	   rc = CodeCheck (p->left, &ltype);
	   if (rc) break;
	   rc = CodeCheck (p->right, &rtype);
	   if (rc) break;
	   rc = CodeCheck (p->third, &ttype);
	   if (rc) break;
	   if (rtype!=ttype) goto TYPERR;
	   type = rtype;
	   break;

	case P_WHILE:
	   rc = CodeCheck (p->left, &ltype);
	   if (rc) break;
	   rc = CodeCheck (p->right, &rtype);
	   if (rc) break;
	   type = P_VOID;
	   break;

	case P_FOR:
	   if (p->left) {
	       rc = CodeCheck (p->left, &ltype);
	       if (rc) break;
	   }
	   rc = CodeCheck (p->right, &rtype);
	   if (rc) break;
	   if (p->third) {
	       rc = CodeCheck (p->third, &ltype);
	       if (rc) break;
	   }
	   if (p->fourth) {
	       rc = CodeCheck (p->fourth, &ltype);
	       if (rc) break;
	   }
	   type = P_VOID;
	   break;

	case P_OR:
	case P_LIS: case P_INT:
	   rc = CodeCheck (p->left, &ltype);
	   if (rc) break;
	   rc = CodeCheck (p->right, &rtype);
	   if (rc) break;
	   if (ltype!=rtype) goto TYPERR;
	   type = ltype;
	   break;

	case P_EQ:  case P_NE: case P_IN: case P_NIN:
	case P_LT:  case P_GT: case P_LE: case P_GE:
	   rc = CodeCheck (p->left, &ltype);
	   if (rc) break;
	   rc = CodeCheck (p->right, &rtype);
	   if (rc) break;
	   if (ltype!=rtype) goto TYPERR;
	   type = P_NUMBER;	  /* 20040812.CS. LZS */
	   break;

	case P_AND:
	   rc = CodeCheck (p->left, &ltype);
	   if (rc) break;
	   rc = CodeCheck (p->right, &rtype);
	   if (rc) break;
	   type = ltype;
	   break;

	case P_SUB:
	case P_MUL: case P_DIV: case P_MOD:
	   rc = CodeCheck (p->left, &ltype);
	   if (rc) break;
	   rc = CodeCheck (p->right, &rtype);
	   if (rc) break;
	   if (ltype!=rtype || ltype!=P_NUMBER) goto TYPERR;
	   type = ltype;
	   break;

    case P_ADD:
	   rc = CodeCheck (p->left, &ltype);
	   if (rc) break;
	   rc = CodeCheck (p->right, &rtype);
	   if (rc) break;
	   if (ltype!=rtype) goto TYPERR;
	   type = ltype;
	   break;

    case P_NOT:
	   rc = CodeCheck (p->left, &ltype);
	   if (rc) break;
	   type = P_NUMBER;
	   break;

    case P_NEG:
	   rc = CodeCheck (p->left, &ltype);
	   if (rc) break;
	   if (ltype!=P_NUMBER) goto TYPERR;
	   type = ltype;
	   break;

    default:
	   fprintf (stderr, "CodeCheck: Invalid operation (code %d)\n",
		    p->type);
	   rc= -1;
    }

VEGE:
    if (ptype!=NULL) *ptype = type;
    return rc;

TYPERR:
    fprintf (stderr, "Type check error in type %d at %p\n",
	     p->type, (void *)p);
    rc = -1;
    type = -1;
    goto VEGE;
}
