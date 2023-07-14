/*
 * display.c
 *
 * Anthony's Editor July 93
 *
 * Copyright 1993, 1993 by Anthony Howe.  All rights reserved.  No warranty.
 */

#include <ctype.h>
#include "header.h"

#ifdef __STDC__
static void dispmsg(void);
static int dispframe(void);
static int dispchar(int, int, int);
#else
static void dispmsg();
static int dispframe();
static int dispchar();
#endif

/*
 * Reverse scan for start of logical line containing offset.
 *
 *	offset <= 0	return	0
 *	0 < offset	return	0 <= lnstart(offset) <= offset
 */
t_point
lnstart(offset)
t_point offset;
{
	int ch;

	/* Make sure that offset maps to a buffer location. */
	if (offset <= 0)
		return (0);

	/* Set up sentinel. */
	ch = *buf;
	*buf = '\n';

	/* Scan backwards for a newline. */
	while (*ptr(--offset) != '\n')
		;

	/* Remove the sentinal. */
	*buf = ch;

	/* Adjust offset provided offset != 0 or that we would
	 * have really found a newline instead of the sentinel.
	 */
	if (offset != 0 || ch == '\n')
		++offset;

	return (offset);
}

/*
 * Forward scan for start of logical line following offset.
 *
 *	offset <= 0		return	1
 *	pos(ebuf) <= offset	return	pos(ebuf)
 *	0 < offset < pos(ebuf)	return	offset < lnnext(offset) <= pos(ebuf)
 */
t_point
lnnext(off)
t_point off;
{
	int ch;
	t_char *p;

	/* Set up sentinal. */
	ch = ebuf[-1];
	ebuf[-1] = '\n';

	/* Scan forwards for newline. */
	while (*(p = ptr(off++)) != '\n')
		;

	/* Remove sentinal. */
	ebuf[-1] = ch;

	/* Return offset immediately following a newline. */
	return (pos(p)+1);
}

/*
 * Forward scan for start of logical line segment containing 'finish'.
 * A segment of a logical line corresponds to a physical screen line.
 */
t_point
segstart(start, finish)
t_point start, finish;
{
	t_char *p;
	int c = 0;
	t_point scan = start;
	while (scan < finish) {
		p = ptr(scan);
		if (*p == '\n') {
			c = 0;
			start = scan+1;
		} else if (COLS <= c) {
			c = 0;
			start = scan;
		}
		++scan;
		c += *p == '\t' ? 8 - (c & 7) : 1;
	}
	return (c < COLS ? start : finish);
}

/*
 * Forward scan for start of logical line segment following 'finish'.
 */
t_point
segnext(start, finish)
t_point start, finish;
{
	t_char *p;
	int c = 0;
	t_point scan = segstart(start, finish);
	for (;;) {
		p = ptr(scan);
		if (ebuf <= p || COLS <= c)
			break;
		++scan;
		if (*p == '\n')
			break;
		c += *p == '\t' ? 8 - (c & 7) : 1;
	}
	return (p < ebuf ? scan : pos(ebuf));
}

/*
 * Move up one screen line.
 */
t_point
upup(off)
t_point off;
{
	t_point curr = lnstart(off);
	t_point seg = segstart(curr, off);
	if (curr < seg)
		off = segstart(curr, seg-1);
	else
		off = segstart(lnstart(curr-1), curr-1);
	return (off);
}

/*
 * Move down one screen line.
 */
t_point
dndn(off)
t_point off;
{
	return (segnext(lnstart(off), off));
}

/*
 * Return the offset of a column on the specified line.
 */
t_point
lncolumn(offset, column)
t_point offset;
int column;
{
	int c = 0;
	t_char *p;
	while ((p = ptr(offset)) < ebuf && *p != '\n' && c < column) {
		c += *p == '\t' ? 8 - (c & 7) : 1;
		++offset;
	}
	return (offset);
}

void
display(fn)
void (*fn) _((void));
{
	dispmsg();
	if (dispframe() || marker != NOMARK)
		dispfull();
	else if (fn != NULL)
		(*fn)();
	move(row, col);
	refresh();
}

/*
 * Full screen and cursor update.
 */
void
dispfull()
{
	int i, j;
	t_char *p;
	t_region r;

	move(textline, 0);
	clrtobot();
	i = textline;
	j = 0;
	epage = page;
	getregion(&r);
	for (;;) {
		if (point == epage) {
			row = i;
			col = j;
		}
		p = ptr(epage);
		if (LINES <= i || ebuf <= p) {
			break;
		}
		if (iscrlf(epage) != 1) {
			if (marker != NOMARK) {
				if (point != epage
				&& r.left <= epage && epage <= r.right) {
					standout();
				} else {
					standend();
				}
			}
			j = dispchar(j, (int) *p, TRUE);
		}
		if (*p == '\n' || COLS <= j) {
			j = 0;
			++i;
		}
		++epage;
	}
	standend();
	if (++i < LINES) {
		mvaddstr(i, 0, getmsg(m_eof));
	}
}

/*
 * Cursor update.
 */
void
dispcursor()
{
	int i, j;
	t_char *p;
	t_point pt;

	i = textline;
	j = 0;
	pt = page;
	for (;;) {
		if (point == pt) {
			row = i;
			col = j;
			break;
		}
		p = ptr(pt);
		if (LINES <= i || ebuf <= p)
			break;
		if (iscrlf(pt) != 1)
			j = dispchar(j, (int) *p, FALSE);
		if (*p == '\n' || COLS <= j) {
			j = 0;
			++i;
		}
		++pt;
	}
	standend();
}

static int
dispframe()
{
	int i, flag = FALSE;

	/* Re-frame the screen with the screen line containing the point
	 * as the first line, when point < page.  Handles the cases of a
	 * backward scroll or moving to the top of file.  pgup() will
	 * move page relative to point so that page <= point < epage.
	 */
	if (point < page) {
		page = segstart(lnstart(point), point);
		flag = TRUE;
	}
	/* Re-frame the whole screen when epage <= point.  Handles the
	 * cases of a forward scroll or redraw.
	 */
	if (epage <= point) {
		/* Find end of screen plus one. */
		page = dndn(point);
		/* Number of lines on the screen depends if we are at the
		 * EOF and how many lines are used for help and status.
		 */
		if (pos(ebuf) <= page) {
			page = pos(ebuf);
			i = LINES-2;
		} else {
			i = LINES;
		}
		i -= textline;
		/* Scan backwards the required number of lines. */
		while (0 < i--)
			page = upup(page);
		flag = TRUE;
	}
	return (flag);
}

static int
dispchar(col, ch, flag)
int col, ch, flag;
{
	if (isprint(ch) || ch == '\t' || ch == '\n') {
		if (flag) {
			/* Handle tab expansion ourselves.  Differences
			 * between historical Curses and NCurses 2021
			 * WRT tab handling I can't suss just yet.
			 *
			 * Historical Curses addch() would handle tab
			 * expansion as I recall while (current 2021)
			 * NCurses does not by default it seems.
			 */
			if (ch == '\t') {
				int t;
				for (t = 8 - (col & 7); 0 < t; t--) {
					addch(' ');
				}
			} else {
				addch(ch);
			}
		}
		col += ch == '\t' ? 8 - (col & 7) : 1;
	} else {
		char *ctrl = printable(ch);
		col += (int) strlen(ctrl);
		if (flag) {
			addstr(ctrl);
		}
	}
	return (col);
}

static void
dispmsg()
{
	int r, c;
	standout();
	move(MSGLINE, 0);
	if (msgflag) {
		addstr(msgline);
		msgflag = FALSE;
	} else if (modified) {
		printw(getmsg(m_modified), filename);
	} else {
		printw(getmsg(m_file), filename, pos(ebuf));
	}
	/* Pad remainder of row with standout spaces, clrtoeol() does not. */
	getyx(stdscr, r, c);
	for ( ; c < COLS; c++){
		addch(' ');
	}
	standend();
}

void
ruler(ncols)
int ncols;
{
	int r, c, col;
	getyx(stdscr, r, c);
	standout();
	for (col = 1; col <= ncols; ++col) {
		switch (col % 10) {
		case 0:
			mvprintw(r, col - (col < 100 ? 2 : 3), "%d", col);
			break;
		case 5:
			addch(':');
			break;
		default:
			addch('.');
		}
	}
	/* Write off the end of the line, curses should auto line wrap.
	 * Using a newline would introduce a blank line after the ruler.
	 * This wasn't required in 1993; suspect difference between
	 * original BSD / SystemV curses and newer implementations.
	 */
	addch(' ');
	standend();
}

/*
 * Convert byte ch into a printable character sequence.
 * This mapping is for ASCII systems.
 */
char *
printable(ch)
unsigned ch;
{
	static char buf[5];
	static char *mapping[] = {
		"^?",
		"^@", "^A", "^B", "^C", "^D", "^E", "^F", "^G",
		"^H", "^I", "^J", "^K", "^L", "^M", "^N", "^O",
		"^P", "^Q", "^R", "^S", "^T", "^U", "^V", "^W",
		"^X", "^Y", "^Z", "^[", "^\\", "^]", "^^", "^_"
	};
	if (ch == 0x7f)
		return (mapping[0]);
	if (0 <= ch && ch < 32)
		return (mapping[ch+1]);
	if (isprint(ch)) {
		buf[0] = ch;
		buf[1] = '\0';
	} else {
		sprintf(buf, "\\%03o", ch);
	}
	return (buf);
}
