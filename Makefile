# $Id: Makefile 190 2002-01-27 21:27:01Z schani $

VERSION = 0.4

all :

dist :
	rm -rf lispreader-$(VERSION)
	mkdir lispreader-$(VERSION)
	mkdir lispreader-$(VERSION)/doc
	cp README COPYING NEWS lispreader-$(VERSION)/
	cp -pr lispreader.[ch] docexample.c comment-test.c lispreader-$(VERSION)/
	cp Makefile.dist lispreader-$(VERSION)/Makefile
	cp doc/{lispreader,version}.texi lispreader-$(VERSION)/doc/
	cp doc/Makefile lispreader-$(VERSION)/doc/
	make -C lispreader-$(VERSION)/doc/
	tar -zcvf lispreader-$(VERSION).tar.gz lispreader-$(VERSION)
	rm -rf lispreader-$(VERSION)

clean :
	rm -f *~
	make -C doc clean
