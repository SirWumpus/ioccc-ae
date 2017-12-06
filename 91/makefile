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
ae: ant.c
	${CC} ${CFLAGS} -DMODE=0600 -DBUF=32767 ant.c -o ae -lcurses -ltermcap


