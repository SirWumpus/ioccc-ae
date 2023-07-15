#
# makefile
#
# Anthony's Editor July 93
#
# Copyright 1993, 1993 by Anthony Howe.  All rights reserved.  No warranty.
#

CC	= cc
CFLAGS	= -DCONFIG='".aerc"' -O0 -g

LD	= cc
LDFLAGS =
LIBS	= -lcurses -ltermcap

CP	= cp
MV	= mv
RM	= rm

E	=
O	= .o

OBJ	= search$O command$(O) data$(O) display$(O) gap$(O) key$(O) main$(O)

all: build

build: ae$(E)

ae$(E) : $(OBJ)
	$(LD) $(LDFLAGS) -o ae$(E) $(OBJ) $(LIBS)

header.h : key.h

command$(O): command.c header.h
	$(CC) $(CFLAGS) -c command.c

data$(O): data.c header.h
	$(CC) $(CFLAGS) -c data.c

display$(O): display.c header.h
	$(CC) $(CFLAGS) -c display.c

gap$(O): gap.c header.h
	$(CC) $(CFLAGS) -c gap.c

key$(O): key.c header.h
	$(CC) $(CFLAGS) -c key.c

main$(O): main.c header.h
	$(CC) $(CFLAGS) -c main.c

search$(O): search.h search.c
	$(CC) $(CFLAGS) -c search.c

search$(E): search.h search.c
	$(CC) $(CFLAGS) -DTEST -o$*$(E) search.c

clean:
	-$(RM) $(OBJ) ae$(E) search$(E) *.core

install:
	-$(MV) ae$(E) $(HOME)/bin/$(HOSTNAME)

