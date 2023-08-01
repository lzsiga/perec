CC = gcc
CFLAGS   += ${COMMON_CFLAGS32}   -g -W -Wall -pedantic
CXXFLAGS += ${COMMON_CXXFLAGS32} -g -W -Wall -pedantic
CPPFLAGS += ${COMMON_CPPFLAGS}   -D __UNIX__ -Idevel/src
YFLAGS = -d -v
LDFLAGS  += ${COMMON_LDFLAGS32} -g -ldl
DIR = perec
TGZ = $(DIR).tgz
HTGZ = $(DIR).host.tgz
TARS = *.y *.c *.h Makefile *.pe
DIRTARS = $(foreach file,$(TARS),$(DIR)/$(file))

all: perec
 
clean:
	rm -f percpars.c *.o 2>/dev/null | true

tar:
	(cd ../; tar czf $(TGZ) $(DIRTARS))
	tgztimer ../$(TGZ)
	ls -l ../$(TGZ)

untar:
	(cd ../; tar xzf $(TGZ))
	make clean

hostuntar: clean
	(cd ../; tar xzf $(HTGZ))
	make clean

pensave: tar
	cp -p ../$(TGZ) /pen
	sync

debug:
	CFLAGS = $(CFLAGS) -D harcsa
	make rsatest

nodebug:
	CFLAGS = $(CFLAGS) -D potyka
	make rsatest

perecefence: LDFLAGS += -lefence
perecefence: perec

perec:  percmain.o percpars.o perc1.o percvar.o \
 	percchk.o percstr.o percmem.o perccalc.o \
 	percblt.o percfun.o devel/lib/devel.a
	$(CC) $(LDFLAGS) -o $@ $^

perec_ifcounter:  percmain.o percpars_ifcounter.o perc1.o percvar.o \
 	percchk.o percstr.o percmem.o perccalc.o \
 	percblt.o percfun.o devel/lib/devel.a
	$(CC) $(LDFLAGS) -o $@ $^

percmain.o, percpars.o, perc1.o, percmem.o: perec.h

%.c: %.y
	bison $(YFLAGS) -o $@ $<

test1: perec
	./perec -quiet /dev/null <test1.pe

iftest: perec_ifcounter
	exec ./perec_ifcounter <iftest.pe /dev/null
