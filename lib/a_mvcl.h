/* a_mvcl.h */ 
 
#ifndef A_MVCL_H
#define A_MVCL_H

#include <stddef.h>

/*
   Az 'MVCL' utasitas meghivasat lehetove tevo assembly rutinok.
 
   a_mvcl: 'egyszeru' mvcl az ismert parameterekkel
   a_mvcr: jobbra illesztett valtozat, numerikus mezokhoz ajanlott
 
   a_mvcl_c: '\0'-val lezart stringet masol.
   a_mvcr_c: '\0'-val lezart stringet masol.
 
   a_mvcl_sql, a_mvcr_sql: az SQL-ben ismert VARCHAR-os, indikatoros
       valtozatok.
 
   a_sql_mvcl, a_sql_mvcr:
       VARCHAR-t es indikatort tolt fel fix mezobol.
*/
 
#define a_mvcl_sql   a_mvcls
#define a_mvcr_sql   a_mvcrs
#define a_sql_mvcl   a_smvcl
#define a_sql_mvcr   a_smvcr
#define a_sql_mvcl_x a_smvclx
#define a_sql_mvcr_x a_smvcrx
 
extern void *a_mvcl (size_t tolen, void *to,
                     size_t fromlen, const void *from, char fill);
extern void *a_mvcr (size_t tolen, void *to,
                     size_t fromlen, const void *from, char fill);
 
extern void *a_mvcl_c (size_t tolen, void *to,
                       const void *fromcstring, char fill);
extern void *a_mvcr_c (size_t tolen, void *to,
                       const void *fromcstring, char fill);
 
extern void *a_c_mvcl (void *tocstring,
                       size_t fromlen, const void *from, char fill);
 
extern void *a_c_mvcr (void *tocstring,
                       size_t fromlen, const void *from, char fill);
 
#define NULL_INDICATOR (-1)    /* a mezo nincs kitoltve (IS NULL) */
#define VALID_INDICATOR 0      /* a mezo ki van toltve */
 
extern void *a_mvcl_sql (size_t tolen, void *to,
                         int indicator, const void *varfrom, char fill);
extern void *a_mvcr_sql (size_t tolen, void *to,
                         int indicator, const void *varfrom, char fill);
 
extern void *a_sql_mvcl (void *tovarchar, short *toind,
                         size_t fromlen, const void *from, char fill);
extern void *a_sql_mvcr (void *tovarchar, short *toind,
                         size_t fromlen, const void *from, char fill);

/* _x verzi�k: megadhat� az output varchar maxim�lis m�rete
   (brutt� m�ret, vagyis a hosszt�nyez�vel egy�tt (azaz>=2)
 */

extern void *a_sql_mvcl_x (size_t tosize,  void *tovarchar, short *toind,
                           size_t fromlen, const void *from, char fill);

extern void *a_sql_mvcr_x (size_t tosize, void *tovarchar, short *toind,
                           size_t fromlen, const void *from, char fill);

/* _y verzi�: mint az '_x', de az inputot a strntrim-mel csonk�tja
    (strntrim-et h�vja), a t�lt�karakter fixen a sz�k�z,
    visszaadott �rt�k az indik�tor
 */

int a_sql_mvcl_y (size_t tosize,  void *tovarchar, short *toind,
                  size_t fromlen, const void *from);

/* _y verzi�: mint az '_c', de az inputot a strntrim-mel csonk�tja
    (strntrim-et h�vja), a t�lt�karakter fixen a sz�k�z
    visszaadott �rt�k a nett� hossz
 */

/* m�g egy v�ltozat a_sql_mvcl_z: a 'tosize' netto ertendo: 
    a_sql_mvcl_z (sizeof myvarchar.arr, &myvarchar, &ind, fromlen, from);
   alternat�va:
    a_sql_mvcl_y (2 + sizeof myvarchar.arr, &myvarchar, &ind, fromlen, from);

  a kavar�s oka az illeszt�si k�vetelm�ny miatti kihaszn�latlan byte:

    sizeof (varchar[1]) != 3
    sizeof (varchar[1]) == sizeof (varchar[2]) == 4
 */
int a_sql_mvcl_z (size_t nettosize, void *tovarchar, short *toind,
                  size_t fromlen, const void *from);

size_t a_mvcl_y (size_t tolen, void *to,
		 size_t fromlen, const void *from);

void a_sql_sql (void *tovarchar, short *toind, const void *fromvarchar, int fromind);

struct ConstBuffData;
struct BuffData;
struct memory_fun;

int a_sql_bd (size_t tomax, void *tovarchar, short *toind,
              const struct ConstBuffData *from);

/* a_bd_sql: memoriat foglal, a 'to' felulirodik */
int a_bd_sql    (struct BuffData *to, int indicator, const void *varfrom);
int a_bd_sql_mf (struct BuffData *to, int indicator, const void *varfrom, const struct memory_fun *mf);

/* tomax: sizeof (tovharchar->arr), teh�t nett�
   megj: a brutt� az illeszt�si k�vetelmeny miatt probl�m�s:
   a varchar[7] �s a varchar[8] ugyanakkora */

int a_sql_c (size_t tomax, void *tovarchar, short *toind,
             const char *from);

/* egy �jabb verzi�: 'strntrim'-et h�v; 'tomax' nett�:

    char strbuff [12] = "Innen";
    varchar myvarchar [33];	-- ide --

    a_sql_maxc (sizeof myvarchar.arr, &myvarchar, &myind, sizeobj (strbuff));
*/

int a_sql_maxc (size_t tomax, void *tovarchar, short *toind,
		size_t frommax, const char *from);

#endif
