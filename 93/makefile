#
# makefile
#
# Anthony's Editor July 93
#
# Copyright 1993, 2023 by Anthony Howe.  All rights reserved.  No warranty.
#

.POSIX :

.SUFFIXES :
O = .o
E =
.SUFFIXES : .c .i $O

TAG	:= $$(git tag -l | sed -n '/^ae93-/p' | sort -n | tail -n1)

CC	= cc
#CFLAGS	= -DTAG="\"${TAG}\"" -Wall -Os

# Debug build
CFLAGS	= -DTAG="\"${TAG}\"" -Wall -O0 -g

LD	= cc
LDFLAGS =

# Cygwin
#LIBS	= -lcurses

# NetBSD
LIBS	= -lcurses -ltermcap

.c$O :
	${CC} ${CFLAGS} ${CPPFLAGS} -c $<

$O$E :
	$(LD) $(LDFLAGS) $(CC_E)$*$E $*$O $(LIBS)

.c$E :
	$(CC) $(CFLAGS) ${CPPFLAGS} $(LDFLAGS) $(CC_E)$*$E $< $(LIBS)

.c.i:
	${CC} -E ${CPPFLAGS} $*.c >$*.i

OBJ	= search$O command$(O) data$(O) display$(O) gap$(O) key$(O) main$(O)

all: build

build: ae$(E)

ae$(E) : $(OBJ)
	$(LD) $(LDFLAGS) -o ae$(E) $(OBJ) $(LIBS)

header.h : key.h

command$(O): search.h header.h command.c
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
	-rm -f $(OBJ) *.core

distclean: clean
	-rm -f ae$(E) search$(E)

install: build
	install -d ${HOME}/bin
	install -p -s -m 0755 ae$E ${HOME}/bin
	install -p -m 0400 mode.rc ${HOME}/.aerc
	@echo
	@echo "Default configuration ~/.aerc is the modal vi style (most portable)."
	@echo "Copy modeless.xterm to ~/.aerc and edit to taste for an emacs style."
	@echo
