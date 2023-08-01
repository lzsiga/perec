/* perec.h */

#ifndef PEREC_H
#define PEREC_H

#include "buffer.h"

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
#define P_NEG 16
#define P_LIS 17
#define P_INT 18
#define P_CONST  19
#define P_VAR	 20
#define P_FUNC	 21
#define P_LET	 22
#define P_IF	 23
#define P_WHILE  24

#define P_SEQ	 30
#define P_FREE	 50

typedef int VType;
#define P_NUMBER 25
#define P_STRING 26

typedef struct Memory {
    int refcount;
    int size;
} Memory;

extern Memory *MemGet	(int size);  /* refcount = 1 */
extern void MemDrop (Memory *); 	/* --refcount */
extern void MemStat (void);

#define MemAddr(m)  ((char *)(m) + sizeof (Memory))
#define MemDropM(m) { if ((m)) MemDrop((m)); }
#define MemUse(m)   { if ((m)) ++(m)->refcount; }

typedef struct Value {
    VType vtype;	   /* P_NUMBER or P_STRING */
    union {
       int i;		  /* integer value */
       BuffData b;	  /* string value */
    } u;
    Memory *mem;	   /* if vtype == P_STRING */
} Value;

extern int ValuePrint (const Value *v);

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

typedef struct Code {
    struct Code *left;	  /* condition in IF */
    struct Code *right;   /* THEN in IF */
    struct Code *third;   /* ELSE in IF */
    short  type;
    Value  value;	  /* if type == P_CONST */
    Variable *var;	  /* if type == P_VAR	*/
    Function *fun;	  /* if type == P_FUNC	*/
} Code;

extern Code *NewNumber (int number);
extern Code *NewUnary  (int type, Code *left);
extern Code *NewBinary (int type,
			  Code *left, Code *right);
extern Code *NewString (const BuffData *from, int option);
extern Code *NewVar    (const BuffData *name);
extern Code *NewFunc   (const BuffData *name, const Code *args);
extern Code *NewLet    (const BuffData *name, const Code *args);
extern Code *NewIf     (const Code *cond, const Code *ifthen,
			const Code *ifelse);
extern Code *NewWhile  (const Code *cond, const Code *code);

extern int CodeCalc  (const Code *p, Value *into);
extern int CodePrint (const Code *p);
extern int CodeCheck (const Code *p, int *type);

extern Code *PercPars (void);

#endif
