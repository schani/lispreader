# $Id: Makefile 178 1999-11-18 23:02:04Z schani $

lisptest : lisptest.c lispparse.c lispparse.h Makefile
	gcc -Wall -g -o lisptest lispparse.c lisptest.c
