PREFIX=/usr/local
OBJS=integrated_ra_receiver.o integrated_ra_receiver_cb.o \
	integrated_ra_receiver_main.o seti_process.o stime.o tp_io.o \
	spec_io.o psr_io.o inter_io.o
systype=$(shell uname -m)
ifeq ($(systype),x86_64)
  RESTRICTED=0
else
  RESTRICTED=1
endif

ifeq ($(RESTRICTED),1)
  CFLAGS=-O3 -g -I/usr/include/X11
else
  CFLAGS=-O3 -g -I/usr/include/X11 -msse -msse2 -msse3 -msse4a -mfpmath=sse
endif
LDFLAGS=-lforms -lm -lflimage -lX11
SOBJS=receiver_start.o receiver_start_cb.o receiver_start_main.o
TOBJS=dtracker.o dtracker_cb.o dtracker_main.o stime.o
PROGS=receiver_start integrated_ra_receiver dtracker
PYPROGS=uhd_ra_receiver_new.py iraconfig.py uhd_ra_interferometer.py
SCRIPTS=ira install_ira
DESKTOP=Ira.desktop Science.directory
ICONS=Ira.png science_icon.png
all: version integrated_ra_receiver receiver_start dtracker uhd_ra_receiver_new.py
clean:
	rm -f *.o integrated_ra_receiver receiver_start exptime makekey installkit send  dtracker *.tar.gz
	rm -f uhd_ra_receiver_new.py
integrated_ra_receiver: $(OBJS)
integrated_ra_receiver_main.o: integrated_ra_receiver_main.c version.h
receiver_start: $(SOBJS)
dtracker: $(TOBJS)
version:
	echo "#define VERSION " \"`date +%Y%m%d-%H:%M:%S`-`uname -m`\" >version.h
uhd_ra_receiver_new.py: uhd_ra_receiver_new.grc
	grcc -d . uhd_ra_receiver_new.grc
install:
	cp ira $(PREFIX)/bin
	cp uhd_ra_receiver_new.py $(PREFIX)/bin
	chmod 755 $(PREFIX)/bin/uhd_ra_receiver_new.py
	cp uhd_ra_interferometer.py $(PREFIX)/bin
	chmod 755 $(PREFIX)/bin/uhd_ra_interferometer.py
	cp receiver_start $(PREFIX)/bin
	cp integrated_ra_receiver $(PREFIX)/bin
	cp dtracker $(PREFIX)/bin
	- cp iraconfig.py $(PREFIX)/lib*/python2.*/site-packages
	- cp iraconfig.py $(PREFIX)/lib*/python2.*/dist-packages
	- mkdir $(PREFIX)/share/doc
	- mkdir $(PREFIX)/share/doc/ira
	cp *.html $(PREFIX)/share/doc/ira
	cp *.png $(PREFIX)/share/doc/ira
	chmod 755 $(PREFIX)/bin/ira
	chmod 755 $(PREFIX)/bin/uhd_ra_receiver_new.py
	chmod 755 $(PREFIX)/bin/receiver_start
	xdg-desktop-menu install --novendor Science.directory Ira.desktop
	xdg-icon-resource install --novendor --size 32 Ira.png Ira
	xdg-icon-resource install --novendor --size 32 science_icon.png Science
	

installkit: $(PROGS) $(PYPROGS) $(SCRIPTS) $(DESKTOP) $(ICONS) Makefile
	tar czvf ira-`uname -m`.tar.gz $(PROGS) $(PYPROGS) $(SCRIPTS) $(DESKTOP) \
	*.html *.png Makefile
	touch installkit
