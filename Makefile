BINDIR=/usr/bin

CFLAGS=-O2 -g
WARNFLAGS=-Wall
powertop: powertop.c config.c Makefile
	gcc ${CFLAGS} ${WARNFLAGS}  powertop.c config.c -o powertop

install: powertop
	cp powertop ${DESTDIR}${BINDIR}
	
clean:
	rm -f *~ powertop
		