/* perc1.c */

#include <stdio.h>
#include <stdlib.h>

#include "defs.h"

#include "perec.h"

Code *NewVar (const BuffData *name)
{
    Code *p;

    p = (Code *)ecalloc (1, sizeof (Code));
    p->type = P_VAR;
    p->var = VarFind (name);
    if (p->var==NULL) {
	fprintf (stderr, "Undefined variable '%.*s'",
		 (int)name->len, name->ptr);
	exit (32);
    }
    return p;
}

Code *NewFunc (const BuffData *name, const Code *args)
{
    Code *p;

    p = (Code *)ecalloc (1, sizeof (Code));
    p->type = P_FUNC;
    p->fun = FunFind (name);
    if (p->fun==NULL) {
	fprintf (stderr, "Undefined function '%.*s'",
		 (int)name->len, name->ptr);
	exit (32);
    }
    p->left= (Code *)args;
    return p;
}

StaticBD(Ninput,  "INPUT");
StaticBD(Nsubstr, "SUBSTR");

Code *NewSect (const Code *pos, const Code *len)
{
    Code *p;

    p = NewBinary (P_LIS, pos, len);
    p = NewBinary (P_LIS, NewVar (&Ninput), p);
    p = NewFunc (&Nsubstr, p);
    return p;
}

Code *NewLet (int op, const BuffData *var, const Code *value)
{
    Code *p;

    p = (Code *)ecalloc (1, sizeof (Code));
    p->type = op; /* P_LET, P_INC, P_DEC */
    p->var = VarFind (var);
    if (p->var==NULL) {
	fprintf (stderr, "Undefined variable '%.*s'",
		 (int)var->len, var->ptr);
	exit (32);
    }
    p->left = (Code *)value;
    return p;
}

Code *NewIf	(const Code *cond, const Code *ifthen,
			const Code *ifelse)
{
    Code *p;
    p= (Code *)ecalloc (1,sizeof (Code));
    p->type = P_IF;
    p->left = (Code*)cond;
    p->right = (Code*)ifthen;
    p->third = (Code*)ifelse;
    return p;
}

Code *NewIfC	(const Code *cond, const Code *ifthen,
			const Code *ifelse)
{
    Code *p;
    p= (Code *)ecalloc (1,sizeof (Code));
    p->type = P_IFC;
    p->left = (Code*)cond;
    p->right = (Code*)ifthen;
    p->third = (Code*)ifelse;
    return p;
}

Code *NewWhile	(const Code *cond, const Code *code)
{
    Code *p;
    p= (Code *)ecalloc (1,sizeof (Code));
    p->type = P_WHILE;
    p->left = (Code*)cond;
    p->right = (Code*)code;
    return p;
}

Code *NewFor  (const Code *before, const Code *cond,
	       const Code *after,  const Code *code)
{
    Code *p;

    p= (Code *)ecalloc (1,sizeof (Code));
    p->type = P_FOR;
    p->left = (Code*)before;
    p->right = (Code*)cond;
    p->third = (Code*)after;
    p->fourth = (Code*)code;
    return p;
}

void CreateVar (const BuffData *name, int type)
{
    Variable *v;

    v = VarNew (name);
    if (v==NULL) {
	fprintf (stderr, "Duplicated variable '%.*s'",
		 (int)name->len, name->ptr);
	exit (32);
    }
    v->value.vtype = type;
    if (type == P_NUMBER) {
	v->value.u.i = 0;
    } else {
	v->value.u.b.len = 0;
	v->value.u.b.ptr = "";
    }
}

Code *NewNumber (int number)
{
    Code *p;

    p = (Code *)ecalloc (1, sizeof (Code));
    p->type = P_CONST;
    p->value.vtype = P_NUMBER;
    p->value.u.i = number;
    return p;
}

Code *NewUnary (int type, Code *left)
{
    Code *p;

    p = (Code *)ecalloc (1, sizeof (Code));
    p->type = type;
    p->left = left;
    return p;
}

Code *NewBinary (int type,
		 const Code *left, const Code *right)
{
    Code *p;

    p = (Code *)ecalloc (1, sizeof (Code));
    p->type = type;
    p->left = (Code *)left;
    p->right = (Code *)right;
    return p;
}

static int CodePrintCore (const Code *p, int indent);

int CodePrint (const Code *p)
{
    return CodePrintCore (p, 1);
}

static void NewLine (int indent)
{
    putc ('\n', stdout);
    for (;indent>0;--indent) fputs ("	 ", stdout);
}

static int CodePrintCore (const Code *p, int indent)
{
    int rc, rrc;
    const Code *first;

    rc = 0;

    if (p == NULL) {
	fputs ("<ures kif>", stdout);
	rc = -1;
	goto VEGE;
    }

    switch (p->type) {

    case P_SEQ:
	rc = 0;
	fputs ("{", stdout);
	do {
	    NewLine (indent);
	    if (p->type == P_SEQ) first= p->left, p= p->right;
	    else		  first= p,	  p= NULL;
	    rrc = CodePrintCore (first, indent);
	    if (rrc) rc = rrc;
	    fputs (";", stdout);
	} while (p!=NULL);
	NewLine (indent-1);
	fputs ("} ", stdout);
	goto VEGE;	       /* 20050301.LZS. */

    case P_CONST:
	ValuePrint (&p->value, 1);
	break;

    case P_VAR:
	fprintf (stdout, "%.*s",
		 (int)p->var->name.len, p->var->name.ptr);
	break;

    case P_LET:
	fprintf (stdout, "%.*s := ",
		 (int)p->var->name.len, p->var->name.ptr);
	rc = CodePrintCore (p->left, indent);
	break;

    case P_INC:
	if (p->left->type == P_CONST &&
	    p->left->value.vtype == P_NUMBER &&
	    p->left->value.u.i == 1) {
	    fprintf (stdout, "++%.*s",
		 (int)p->var->name.len, p->var->name.ptr);
	    rc = 0;
	} else {
	    fprintf (stdout, "%.*s += ",
		 (int)p->var->name.len, p->var->name.ptr);
	    rc = CodePrintCore (p->left, indent);
	}
	break;

    case P_INCPOST:
	if (p->left->type == P_CONST &&
	    p->left->value.vtype == P_NUMBER &&
	    p->left->value.u.i == 1) {
	    fprintf (stdout, "%.*s++",
		 (int)p->var->name.len, p->var->name.ptr);
	    rc = 0;
	} else {
	    fprintf (stdout, "%.*s += ",
		 (int)p->var->name.len, p->var->name.ptr);
	    rc = CodePrintCore (p->left, indent);
	}
	break;

    case P_DEC:
	if (p->left->type == P_CONST &&
	    p->left->value.vtype == P_NUMBER &&
	    p->left->value.u.i == 1) {
	    fprintf (stdout, "--%.*s",
		 (int)p->var->name.len, p->var->name.ptr);
	    rc = 0;
	} else {
	    fprintf (stdout, "%.*s -= ",
		 (int)p->var->name.len, p->var->name.ptr);
	    rc = CodePrintCore (p->left, indent);
	}
	break;

    case P_DECPOST:
	if (p->left->type == P_CONST &&
	    p->left->value.vtype == P_NUMBER &&
	    p->left->value.u.i == 1) {
	    fprintf (stdout, "%.*s--",
		 (int)p->var->name.len, p->var->name.ptr);
	    rc = 0;
	} else {
	    fprintf (stdout, "%.*s -= ",
		 (int)p->var->name.len, p->var->name.ptr);
	    rc = CodePrintCore (p->left, indent);
	}
	break;

    case P_IF:
	fputs("IF (", stdout);
	rc = CodePrintCore (p->left, indent);
	fputs (") ", stdout);
	rc = CodePrintCore (p->right, indent+1);
	if (p->third) {
	  NewLine (indent);
	  fputs ("ELSE ", stdout);
	  if (p->third->type == P_IF) --indent; /* else if */
	  rc = CodePrintCore (p->third, indent+1);
	}
/*	putc ('}', stdout); */
	break;

    case P_IFC:
	fputs ("((", stdout);
	rc = CodePrintCore (p->left, indent);
	fputs (") ? ", stdout);
	rc = CodePrintCore (p->right, indent);
	fputs (" : ", stdout);
	rc = CodePrintCore (p->third, indent);
	putc (')', stdout);
	break;

    case P_WHILE:
	fputs("WHILE (", stdout);
	rc = CodePrintCore (p->left, indent+1);
	fputs (") ", stdout);
	rc = CodePrintCore (p->right, indent+1);
/*	putc ('}', stdout); */
	break;

    case P_FOR:
	fputs("FOR (", stdout);
	if (p->left)
	    rc = CodePrintCore (p->left, indent+1);
	fputs ("; ", stdout);
	rc = CodePrintCore (p->right, indent+1);
	fputs ("; ", stdout);
	if (p->third)
	    rc = CodePrintCore (p->third, indent+1);
	fputs(") ", stdout);
	rc = CodePrintCore (p->fourth, indent+1);
/*	putc ('}', stdout); */
	break;

    case P_FUNC:
	fprintf(stdout, "%.*s (", (int)p->fun->name.len, p->fun->name.ptr);
	if (p->left) {
	    rc = CodePrintCore (p->left, indent);
	}
	putc (')', stdout);
	break;

    case P_AND: case P_OR:
    case P_EQ:	case P_NE: case P_IN: case P_NIN:
    case P_LT:	case P_GT: case P_LE: case P_GE:
    case P_ADD: case P_SUB:
    case P_MUL: case P_DIV: case P_MOD:
	putc ('(', stdout);
	rc = CodePrintCore (p->left, indent);
	break;

    case P_NOT: case P_NEG:
	putc ('(', stdout);
	break;

    case P_LIS: case P_INT:
	rc = CodePrintCore (p->left, indent);
	break;

    default:
	fputs ("<hibas kif>", stdout);
	rc = -1;
    }

    switch (p->type) {
    case P_AND: fputs (" AND ", stdout); break;
    case P_OR:	fputs (" OR ", stdout);  break;
    case P_NOT: fputs ("NOT ", stdout);  break;
    case P_NEG: fputs ("-", stdout);  break;
    case P_IN:	fputs (" IN ", stdout); break;
    case P_NIN: fputs (" NOT IN ", stdout); break;
    case P_EQ:	fputs (" == ", stdout); break;
    case P_NE:	fputs (" <> ", stdout); break;
    case P_LT:	fputs (" < ", stdout); break;
    case P_GT:	fputs (" > ", stdout); break;
    case P_LE:	fputs (" <= ", stdout); break;
    case P_GE:	fputs (" >= ", stdout); break;
    case P_ADD: fputs (" + ", stdout); break;
    case P_SUB: fputs (" - ", stdout); break;
    case P_MUL: fputs (" * ", stdout); break;
    case P_DIV: fputs (" / ", stdout); break;
    case P_MOD: fputs (" % ", stdout); break;
    case P_INT: fputs (" .. ", stdout); break;
    default: break;
    }

    switch (p->type) {
    case P_NOT: case P_NEG:
	rc = CodePrintCore (p->left, indent);
	putc (')', stdout);
	break;
    case P_AND: case P_OR:
    case P_EQ:	case P_NE:
    case P_LT:	case P_GT:  case P_LE:	case P_GE:
    case P_ADD: case P_SUB:
    case P_MUL: case P_DIV: case P_MOD:
	rrc = CodePrintCore (p->right, indent);
	putc (')', stdout);
	rc = rc || rrc;
	break;
    case P_INT:
	rrc = CodePrintCore (p->right, indent);
	rc = rc || rrc;
	break;
    case P_LIS:
	if (p->right) {
	    fputs (",", stdout);
	    rrc = CodePrintCore (p->right, indent);
	    rc = rc || rrc;
	}
	break;
    case P_IN: case P_NIN:
	fputs ("(", stdout);
	if (p->right) {
	    rrc = CodePrintCore (p->right, indent);
	    rc = rc || rrc;
	}
	fputs ("))", stdout);
	break;
   default:
	break;
    }

VEGE:
    return rc;
}

int ValuePrint (const Value *v, int quote)
{
    int rc;

    if (v->vtype==P_NUMBER) {
	rc = fprintf (stdout, "%ld", v->u.i);
    } else {
	rc = fprintf (stdout, (quote&1) ? "'%.*s'" : "%.*s",
		      (int)v->u.b.len, v->u.b.ptr);
    }
    return rc;
}
