BINDIR=/usr/bin

WARNFLAGS=-Wall
CFLAGS=-O2 -g ${WARNFLAGS}
powertop: powertop.c config.c process.c Makefile powertop.h
	gcc ${CFLAGS}  powertop.c config.c process.c -o powertop

install: powertop
	mkdir -p ${DESTDIR}${BINDIR}
	cp powertop ${DESTDIR}${BINDIR}
	
clean:
	rm -f *~ powertop