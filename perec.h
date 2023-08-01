/* perec.h */

#ifndef PEREC_H
#define PEREC_H

#include "buffdata.h"
#include "szio.h"

#define P_AND 1
#define P_OR  2
#define P_NOT 3
#define P_EQ  4
#define P_NE  5
#define P_LT  6
#define P_LE  7
#define P_GT  8
#define P_GE  9
#define P_IN  10
#define P_NIN 11
#define P_ADD 12
#define P_SUB 13
#define P_MUL 14
#define P_DIV 15
#define P_MOD 16
#define P_NEG 17
#define P_LIS 18
#define P_INT 19
#define P_CONST  20
#define P_VAR	 21
#define P_FUNC	 22

#define P_LET	 23
#define P_INC	 24
#define P_DEC	 25
#define P_INCPOST 26
#define P_DECPOST 27

#define P_IF	 30
#define P_IFC	 31	 /* expr ? expr : expr */
#define P_WHILE  32
#define P_FOR	 33

#define P_SEQ	 40
#define P_FREE	 50

typedef int VType;
#define P_NUMBER 61
#define P_STRING 62
#define P_VOID	 60
#define P_ANY	 63

typedef struct Memory {
    int refcount;
    int size;
} Memory;

extern Memory *MemGet	(int size);  /* refcount = 1 */
extern void MemDrop (Memory **);     /* --refcount */
extern void MemStat (int opt);	     /* opt==1: dont print if no mem used */

#define MemAddr(m)   ((char *)(m) + sizeof (Memory))
#define MemDropM(mp) { if (*(mp)) MemDrop((mp)); }
#define MemUse(mp)   { if (*(mp)) ++((*(mp))->refcount); }

typedef struct Value {
    VType vtype;	   /* P_NUMBER or P_STRING */
    union {
       long i;		  /* integer value */
       BuffData b;	  /* string value */
    } u;
    Memory *mem;	   /* if vtype == P_STRING */
} Value;

extern int ValuePrint (const Value *v, int quote);

typedef struct Variable {
    BuffData name;
    Value value;
} Variable;

extern Variable *VarFind (const BuffData *name);
extern Variable *VarNew  (const BuffData *name);

extern void CreateVar (const BuffData *name, VType type);

struct Code;

typedef int (*BuiltIn)(Value *retval, int argc, Value *args);

typedef struct Function {
    BuffData name;
    VType rtype;	    /* type of return value */
    int minargc;	    /* minimum number of arguments */
    int maxargc;	    /* maximum number of arguments */
    int atypno; 	    /* number of argument types >= 1 */
    VType *atype;	    /* type of arguments (atypno) */
    BuiltIn bltin;
} Function;

extern Function *FunFind (const BuffData *name);
extern Function *FunNew  (const BuffData *name);

void BltInit (void);

int BStrCat (Value *retval, int argc, Value *args);

typedef struct Code {	  /* IF    WHILE  FOR	  */
    struct Code *left;	  /* cond  cond   before  */
    struct Code *right;   /* THEN  CODE   cond	  */
    struct Code *third;   /* ELSE	  after   */
    struct Code *fourth;  /* ----	  CODE	  */
    short  type;
    Value  value;	  /* if type == P_CONST */
    Variable *var;	  /* if type == P_VAR	*/
    Function *fun;	  /* if type == P_FUNC	*/
} Code;

extern Code *NewNumber (int number);
extern Code *NewUnary  (int type, Code *left);
extern Code *NewBinary (int type,
			const Code *left, const Code *right);
extern Code *NewString (const BuffData *from, int option);
extern Code *NewVar    (const BuffData *name);
extern Code *NewFunc   (const BuffData *name, const Code *args);
extern Code *NewSect   (const Code *pos, const Code *len);
extern Code *NewLet    (int op, const BuffData *name, const Code *args);
extern Code *NewIf     (const Code *cond, const Code *ifthen,
			const Code *ifelse);
extern Code *NewIfC    (const Code *cond, const Code *ifthen,
			const Code *ifelse);
extern Code *NewWhile  (const Code *cond, const Code *code);
extern Code *NewFor    (const Code *before, const Code *cond,
			const Code *after, const Code *code);

extern int CodeCalc  (const Code *p, Value *into);
extern int CodePrint (const Code *p);
extern int CodeCheck (const Code *p, int *type);

typedef struct Codes {
    Code *begin;
    Code *main;
    Code *end;
} Codes;

extern int PercPars (SZIO_FILE_ID *id, Codes *c);

extern int OutPut (int fileno, void *vrec);

#endif
