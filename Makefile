BINDIR=/usr/bin
WARNFLAGS=-Wall
CFLAGS?=-O2 -g ${WARNFLAGS}
CC?=gcc

powertop: powertop.c config.c process.c misctips.c bluetooth.c display.c Makefile powertop.h
	$(CC) ${CFLAGS}  powertop.c config.c process.c misctips.c bluetooth.c display.c -lncurses -o powertop

install: powertop
	mkdir -p ${DESTDIR}${BINDIR}
	cp powertop ${DESTDIR}${BINDIR}
	
clean:
	rm -f *~ powertop
