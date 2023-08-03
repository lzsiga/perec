#!/bin/sh

# the program comes from standard input
# the inputfile is the first parameter

../perec -quiet <<'DONE' reverse.sh
STRING S;
S := STRREV(INPUT);
PUT(S);
DONE
