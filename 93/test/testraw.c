/*
 * cc -otestraw testraw.c -lcurses -ltermcap
 */

#include <stdio.h>
#include <curses.h>
#include <termios.h>
#include <unistd.h>

#define linemode(flag)	(mode ? noraw() : raw())

int
main(int argc, char **argv)
{
	struct termios term;

	(void) tcgetattr(fileno(stdin), &term);
	(void) printf("before curses c_lflag=%lx vmin=%d vtime=%d\n", (long)term.c_lflag, term.c_cc[VMIN], term.c_cc[VTIME]);

	sleep(3);
	initscr();
	raw();

	(void) tcgetattr(fileno(stdin), &term);
	(void) printf("raw c_lflag=%lx vmin=%d vtime=%d\n", (long)term.c_lflag, term.c_cc[VMIN], term.c_cc[VTIME]);

	noraw();

	(void) tcgetattr(fileno(stdin), &term);
	(void) printf("noraw c_lflag=%lx vmin=%d vtime=%d\n", (long)term.c_lflag, term.c_cc[VMIN], term.c_cc[VTIME]);

	endwin();
	putchar('\n');

	return 0;
}
