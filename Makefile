# $Id: Makefile 181 1999-12-21 12:54:22Z schani $

CC=gcc
CFLAGS=-Wall -g -I.

all : docexample

docexample : docexample.o lispreader.o
	$(CC) -Wall -g -o docexample lispreader.o docexample.o

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean :
	rm -f docexample *.o *~
