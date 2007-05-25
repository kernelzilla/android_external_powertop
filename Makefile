BINDIR=/usr/bin
LOCALESDIR=/usr/share/locale
MANDIR=/usr/share/man/man1
WARNFLAGS=-Wall 
CFLAGS?=-O2 -g ${WARNFLAGS}
CC?=gcc

powertop: powertop.c config.c process.c misctips.c bluetooth.c display.c suggestions.c wireless.c Makefile powertop.h
	$(CC) ${CFLAGS}  powertop.c config.c process.c misctips.c bluetooth.c display.c suggestions.c wireless.c -lncurses -o powertop
	@(cd po/ && $(MAKE))

powertop.1.gz: powertop.1
	gzip -c $< > $@

install: powertop powertop.1.gz
	mkdir -p ${DESTDIR}${BINDIR}
	cp powertop ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${MANDIR}
	cp powertop.1.gz ${DESTDIR}${MANDIR}
	@(cd po/ && env LOCALESDIR=$(LOCALESDIR) DESTDIR=$(DESTDIR) $(MAKE) $@)

# This is for translators. To update your po with new strings, do :
# svn up ; make uptrans LG=fr # or de, ru, hu, it, ...
uptrans:
	xgettext -C -s -k_ -o po/powertop.pot *.c *.h
	@(cd po/ && env LG=$(LG) $(MAKE) $@)

clean:
	rm -f *~ powertop powertop.1.gz po/powertop.pot
	@(cd po/ && $(MAKE) $@)

