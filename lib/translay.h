/* translay.h */

#ifndef TRANSLAY_H
#define TRANSLAY_H

#include <stddef.h>

void Translay (const unsigned char map[256], size_t len, void *add);
void TransMov (const unsigned char map[256], void *to,
               size_t len, const void *from);

void TransMovX (const unsigned char map[256],
                size_t tolen,   void *to,
                size_t fromlen, const void *from,
                char fill);

extern const unsigned char asc2ebc7 [256];
extern const unsigned char ebc2asc7 [256];

extern const unsigned char lat_a2e [256];
extern const unsigned char lat_e2a [256];

extern const unsigned char lat_l2u [256];
extern const unsigned char lat_u2l [256];

extern const unsigned char vidi2lat [256];
extern const unsigned char lat2vidi [256];

/* lat2rsol: latin2-rõl RSO-printerre, kisbetûket is */
extern const unsigned char lat2rsol [256];

extern const unsigned char tr15to25 [256];
extern const unsigned char tr25to15 [256];

extern const unsigned char lat_ftpc [256]; /* 852/CWI javitasa FTP utan */

extern const unsigned char lat_ekt  [256]; /* ékezettelenít */
extern const unsigned char lat_ektu [256]; /* ékezettelenít és nagybetûsít */

extern const unsigned char lat_rov [256]; /* íóõúûÍÓÕÛ->ioöüIOÖÜ */

extern const unsigned char huntr  [256];
/* mindent szóközzé, ami nem A-ZÁÉÍÓÖÕÚÜÛÄ */

extern const unsigned char mnevtr  [256];
/* mindent szóközzé, ami nem A-ZÁÉÍÓÖÕÚÜÛÄ.-' */

extern const unsigned char lat2dos [256]; /* latin2 -> cp852  */
extern const unsigned char dos2lat [256]; /* cp852  -> latin2 */

extern const unsigned char x32tr [256]; /* ebcdic: ' ' -> x'32'
                                           ascii:  ' ' -> x'16' */

extern const unsigned char x0d15tr [256];
    /* ebcdic: x'0d' -> x'32', x'15' -> x'35', ' ' -> x'32'
       ascii:  x'0d' -> x'16', x'0a' -> x'95', ' ' -> x'16' */

extern const unsigned char wovel2u [256];
/* minden magánhangzóból aláhúzást (SQL-ben joker-karakter) */

extern const unsigned char lat2type [256]; /* ISO-8859-2 */
#define Lat2type_EnUpper   1 /* A-Z */
#define Lat2type_EnLower   2 /* a-z */
#define Lat2type_EnLetter  3
#define Lat2type_HUpper    4 /* ÁÉÍÓÖÕÚÜÛÄ */
#define Lat2type_HLower    8 /* áéíóöõúüûä */
#define Lat2type_HLetter  12
#define Lat2type_hUpper    5 /* A-Z ÁÉÍÓÖÕÚÜÛÄ */
#define Lat2type_hLower   10 /* a-z áéíóöõúüûä */
#define Lat2type_hLetter  15
#define Lat2type_XUpper   64 /* every uppercase letter in 0xa0..0xff */
#define Lat2type_XLower  128 /* every lowercase letter in 0xa0..0xff */
#define Lat2type_XLetter 192 /* every letter between 0xa0..0xff */
#define Lat2type_xUpper   65 /* every uppercase letter in 0x00..0xff */
#define Lat2type_xLower  130 /* every lowercase letter in 0x00..0xff */
#define Lat2type_xLetter 195 /* every letter between 0xa0..0xff */

#define Lat2type_Digit    16
#define Lat2type_HexDigit 32

#endif
