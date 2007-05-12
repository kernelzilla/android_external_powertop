BINDIR=/usr/bin

powertop: powertop.c config.c Makefile
	gcc -Wall -O2 -g powertop.c config.c -o powertop

install: powertop
	cp powertop ${DESTDIR}${BINDIR}
	
clean:
	rm -f *~ powertop
		