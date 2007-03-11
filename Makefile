# $Id: Makefile 913 2007-03-11 22:41:11Z schani $

VERSION = 0.5

all : lispreader.a

lispreader.a :
	$(MAKE) -f Makefile.dist

dist :
	rm -rf lispreader-$(VERSION)
	mkdir lispreader-$(VERSION)
	mkdir lispreader-$(VERSION)/doc
	cp README COPYING NEWS lispreader-$(VERSION)/
	cp -pr lispreader.[ch] lispscan.h allocator.[ch] pools.[ch] docexample.c lispreader-$(VERSION)/
	cp Makefile.dist lispreader-$(VERSION)/Makefile
	cp doc/{lispreader,version}.texi lispreader-$(VERSION)/doc/
	cp doc/Makefile lispreader-$(VERSION)/doc/
	make -C lispreader-$(VERSION)/doc/
	tar -zcvf lispreader-$(VERSION).tar.gz lispreader-$(VERSION)
	rm -rf lispreader-$(VERSION)

clean :
	rm -f *~
	make -C doc clean
