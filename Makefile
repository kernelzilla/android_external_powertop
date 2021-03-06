VERSION = 1.13

BINDIR=/usr/bin
LOCALESDIR=/usr/share/locale
MANDIR=/usr/share/man/man8
WARNFLAGS=-Wall -Wshadow -W -Wformat -Wimplicit-function-declaration -Wimplicit-int
CFLAGS?=-O1 -g ${WARNFLAGS}
CC?=gcc

CFLAGS+=-D VERSION=\"$(VERSION)\"

# 
# The w in -lncursesw is not a typo; it is the wide-character version
# of the ncurses library, needed for multi-byte character languages
# such as Japanese and Chinese etc.
#
# On Debian/Ubuntu distros, this can be found in the
# libncursesw5-dev package. 
#

OBJS = powertop.o config.o process.o misctips.o bluetooth.o display.o suggestions.o wireless.o cpufreq.o \
	sata.o xrandr.o ethernet.o cpufreqstats.o usb.o urbnum.o intelcstates.o wifi-new.o perf.o \
	alsa-power.o ahci-alpm.o dmesg.o devicepm.o
	

powertop: $(OBJS) Makefile powertop.h
	$(CC) ${CFLAGS} $(LDFLAGS) $(OBJS) -lncursesw -o powertop
	@(cd po/ && $(MAKE))

powertop.8.gz: powertop.8
	gzip -c $< > $@

install: powertop powertop.8.gz
	mkdir -p ${DESTDIR}${BINDIR}
	cp powertop ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${MANDIR}
	cp powertop.8.gz ${DESTDIR}${MANDIR}
	@(cd po/ && env LOCALESDIR=$(LOCALESDIR) DESTDIR=$(DESTDIR) $(MAKE) $@)
	
valgrind: powertop
	 sudo valgrind ./powertop -d -t 5 1> /dev/null

# This is for translators. To update your po with new strings, do :
# svn up ; make uptrans LG=fr # or de, ru, hu, it, ...
uptrans:
	@(cd po/ && env LG=$(LG) $(MAKE) $@)

clean:
	rm -f *~ powertop powertop.8.gz po/powertop.pot DEADJOE svn-commit* *.o *.orig 
	@(cd po/ && $(MAKE) $@)


dist:
	git tag v$(VERSION)
	git archive --format=tar --prefix="powertop-$(VERSION)/" v$(VERSION) | \
		gzip > powertop-$(VERSION).tar.gz
