BINDIR=/usr/bin

WARNFLAGS=-Wall
CFLAGS=-O2 -g ${WARNFLAGS}
powertop: powertop.c config.c Makefile
	gcc ${CFLAGS}  powertop.c config.c -o powertop

install: powertop
	cp powertop ${DESTDIR}${BINDIR}
	
clean:
	rm -f *~ powertop
		