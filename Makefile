# $Id: Makefile 182 1999-12-21 16:55:25Z schani $

VERSION = 0.1

all :

dist :
	rm -rf lispreader-$(VERSION)
	mkdir lispreader-$(VERSION)
	mkdir lispreader-$(VERSION)/doc
	cp README COPYING lispreader-$(VERSION)/
	cp -pr lispreader.[ch] docexample.c lispreader-$(VERSION)/
	cp Makefile.dist lispreader-$(VERSION)/Makefile
	cp doc/{lispreader,version}.texi lispreader-$(VERSION)/doc/
	cp doc/Makefile lispreader-$(VERSION)/doc/
	make -C lispreader-$(VERSION)/doc/
	tar -zcvf lispreader-$(VERSION).tar.gz lispreader-$(VERSION)
	rm -rf lispreader-$(VERSION)

clean :
	rm -f *~
	make -C doc clean
