/* sajatdump.h */

#ifndef SAJATDUMP_H
#define SAJATDUMP_H

#define sajatdump() (sajatdump1 (1))

void sajatdump1 (int flags);
/* (flags&1)==1: like sajatdump -- write core file and exit */
/* (flags&1)==0: write core file and keep working */

#define SAJATDUMP_HALT	      1
#define SAJATDUMP_KEEPWORKING 0

#endif
