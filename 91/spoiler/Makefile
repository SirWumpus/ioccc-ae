#
# Anthony's Editor
#
# IOCCC 1991
#
# Winner Best Utility
#

all: ae

clean:
	-rm -f ae

# NOTE: some systems may need:
#
#  old SYS V:
#	cc ${CFLAGS} -DMODE=0600 -DBUF=32767 -o ant ant.c -lcurses
#  ATARI ST:
#	cc -DMODE=0 -DBUF=32767 -o ant.ttp ant.c curses.a
#  PC class:
#	wcl -mc -DMODE=0 -DBUF=32767 ant.c curses.lib
#
# This older version may generate lots of warnings with newer compilers.
#
ae: ae.c
	${CC} ${CFLAGS} -DMODE=0600 -DBUF=32767 ae.c -o ae -lcurses -ltermcap

# Mostly cosmetic changes to read like ANSI C 89.
ae-c89: ae-c89.c
	${CC} ${CFLAGS} -DMODE=0600 ae-c89.c -o ae -lcurses -ltermcap
