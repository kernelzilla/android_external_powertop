BINDIR=/usr/bin
WARNFLAGS=-Wall
CFLAGS=-O2 -g ${WARNFLAGS}
CC=gcc

powertop: powertop.c config.c process.c Makefile powertop.h
	$(CC) ${CFLAGS}  powertop.c config.c process.c -o powertop

install: powertop
	install -d ${DESTDIR}${BINDIR}
	cp powertop ${DESTDIR}${BINDIR}
	
clean:
	rm -f *~ powertop
