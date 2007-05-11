BINDIR=/usr/bin

powertop: powertop.c config.c Makefile
	gcc -Wall -O2 powertop.c config.c -o powertop

install: powertop
	cp powertop ${BINDIR}
	
clean:
	rm -f *~ powertop
		