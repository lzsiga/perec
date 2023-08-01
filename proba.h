#ifndef BISON_PROBA_H
# define BISON_PROBA_H

# ifndef YYSTYPE
#  define YYSTYPE int
#  define YYSTYPE_IS_TRIVIAL 1
# endif
# define	IDENT	257
# define	NUMBER	258
# define	STRING	259
# define	NOT	260
# define	AND	261
# define	OR	262
# define	IN	263
# define	IF	264
# define	ELSE	265
# define	WHILE	266
# define	FOR	267
# define	A	268
# define	E	269
# define	C	270
# define	X	271
# define	KNUMBER	272
# define	KSTRING	273


extern YYSTYPE yylval;

#endif /* not BISON_PROBA_H */
