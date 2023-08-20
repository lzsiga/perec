#!/bin/sh

# the program comes from standard input
# the inputfile is the first parameter

echo 'Testing normal parser'
../perec <<'DONE' /dev/null
BEGIN { PUT(1-2-3-4-5); }
DONE

echo 'Testing naive parser'
../perec -naive-parser <<'DONE' /dev/null
BEGIN { PUT(1-2-3-4-5); }
DONE
