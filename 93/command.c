/*
 * command.c
 *
 * Anthony's Editor July 93
 *
 * Copyright 1993, 2023 by Anthony Howe.  All rights reserved.  No warranty.
 */

#include <ctype.h>
#include "header.h"

void
promptmsg(t_msg m)
{
	standout();
	mvaddstr(MSGLINE, 0, getmsg(m));
	standend();
	clrtoeol();
	addch(' ');
	refresh();
}

void
prompt(t_msg m, char *buf, size_t len)
{
	promptmsg(m);
	getinput(buf, len);
}

int
yesno(int flag)
{
	int ch;

	addstr(getmsg(flag ? p_yes : p_no));
	refresh();
	ch = getliteral();
	if (ch == '\r' || ch == '\n') {
		return (flag);
	}

	return (ch == getmsg(p_yes)[1]);
}

/*
 * Return true if more should continue.
 */
int
more(int row)
{
	int ch;

	if (0 < row % (LINES-1)) {
		return (TRUE);
	}
	standout();
	addstr(getmsg(p_more));
	standend();
	clrtoeol();
	refresh();
	ch = getliteral();
	addch('\r');
	clrtoeol();
	return (ch != getmsg(p_quit)[1] && ch != getmsg(p_no)[1]);
}

void
top(void)
{
	inserting = FALSE;
	point = 0;
}

void
bottom(void)
{
	inserting = FALSE;
	epage = point = pos(ebuf);
}

void
quit_ask(void)
{
	if (modified) {
		standout();
		mvaddstr(MSGLINE, 0, getmsg(p_notsaved));
		standend();
		clrtoeol();
		if (!yesno(FALSE)) {
			return;
		}
	}
	quit();
}

void
quit(void)
{
	done = 1;
}

void
redraw(void)
{
	int col;

	clear();
	if (textline != HELPLINE) {
		move(HELPLINE, 0);
		addstr(getmsg(m_help));
		ruler(COLS);
		getyx(stdscr, textline, col);
	}
	display(dispfull);
}

void
left(void)
{
	inserting = FALSE;
	if (0 < point && iscrlf(--point) == 2) {
		--point;
	}
}

void
right(void)
{
	inserting = FALSE;
	if (point < pos(ebuf) && iscrlf(point++) == 1) {
		++point;
	}
}

void
up(void)
{
	inserting = FALSE;
	point = lncolumn(upup(point), col);
	if (iscrlf(point) == 2) {
		--point;
	}
}

void
down(void)
{
	inserting = FALSE;
	point = lncolumn(dndn(point), col);
	if (iscrlf(point) == 2) {
		--point;
	}
}

void
lnbegin(void)
{
	inserting = FALSE;
	point = segstart(lnstart(point), point);
}

void
lnend(void)
{
	point = dndn(point);
	left();
}

void
wleft(void)
{
	inserting = FALSE;
	while (!isalnum(*ptr(--point)) && 0 < point) {
		;
	}
	while (isalnum(*ptr(--point)) && 0 <= point) {
		;
	}
	++point;
}

void
wright(void)
{
	inserting = FALSE;
	t_point epoint = pos(ebuf);
	while (isalnum(*ptr(point)) && point < epoint) {
		++point;
	}
	while (!isalnum(*ptr(point)) && point < epoint) {
		++point;
	}
}

void
pgdown(void)
{
	inserting = FALSE;
	page = point = upup(epage);
	while (textline < row--) {
		down();
	}
	epage = pos(ebuf);
}

void
pgup(void)
{
	int i = LINES;
	inserting = FALSE;
	while (textline < --i) {
		page = upup(page);
		up();
	}
}

void
insert(void)
{
	point = movegap(point);
	if (gap == egap && !growgap(CHUNK)) {
		return;
	}
	if (!inserting){
		undoset();
		inserting = TRUE;
	}
	*gap++ = input == K_LITERAL ? getliteral() : input;
	if (input == '\r' && (gap < egap || growgap(CHUNK))) {
		*gap++ = '\n';
	}
	modified = TRUE;
	point = pos(egap);
}

void
insert_mode(void)
{
	int ch;
	t_point opoint;
	point = opoint = movegap(point);
	undoset();
	while ((ch = getkey(key_mode)) != K_INSERT_EXIT) {
		if (ch == K_STTY_ERASE) {
			if (opoint < point) {
				if (*--gap == '\n'
				&& buf < gap && gap[-1] == '\r') {
					--gap;
				}
				modified = TRUE;
			}
		} else if (ch == K_STTY_KILL) {
			/* Discard current input.*/
			gap -= point - opoint;
		} else {
			assert(gap <= egap);
			if (gap == egap && !growgap(CHUNK)) {
				break;
			}
			*gap++ = ch == K_LITERAL ? getliteral() : ch;
			if (ch == '\r' && (gap < egap || growgap(CHUNK))) {
				*gap++ = '\n';
			}
			modified = TRUE;
		}
		point = pos(egap);
		display(dispfull);
	}
}

void
backsp(void)
{
	if (marker != NOMARK && marker < point) {
		cut();
		return;
	}
	point = movegap(point);
	if (buf < gap) {
		if (*--gap == '\n' && buf < gap && gap[-1] == '\r') {
			--gap;
		}
		point = pos(egap);
		modified = TRUE;
	}
}

void
delete(void)
{
	if (marker != NOMARK && point < marker) {
		cut();
		return;
	}
	point = movegap(point);
	if (egap < ebuf) {
		if (*egap++ == '\r' && egap < ebuf && *egap == '\n') {
			++egap;
		}
		point = pos(egap);
		modified = TRUE;
	}
}

void
readfile(void)
{
	temp[0] = '\0';
	prompt(p_read, temp, BUFSIZ);
	(void) load(temp);
	if (filename[0] == '\0') {
		(void) strncpy(filename, temp, BUFSIZ);
		inserting = FALSE;
		modified = FALSE;
	}
}

void
writefile(void)
{
	standout();
	if (marker == NOMARK || point == marker) {
		strcpy(temp, filename);
		prompt(p_write, temp, BUFSIZ);
	} else {
		temp[0] = '\0';
		prompt(p_bwrite, temp, BUFSIZ);
	}
	(void) save(temp);
	if (marker == NOMARK && filename[0] == '\0')
		strcpy(filename, temp);
}

void
help(void)
{
	textline = textline == HELPLINE ? -1 : HELPLINE;
	/* When textline != HELPLINE, then redraw() will compute the
	 * actual textline that follows the help text.
	 */
	redraw();
}

void
block(void)
{
	marker = marker == NOMARK ? point : NOMARK;
}

void
cut(void)
{
	if (marker == NOMARK || point == marker) {
		return;
	}
	if (scrap != NULL) {
		free(scrap);
		scrap = NULL;
	}
	if (point < marker) {
		(void) movegap(point);
		nscrap = marker - point;
	} else {
		(void) movegap(marker);
		nscrap = point - marker;
	}
	if ((scrap = (t_char*) malloc(nscrap)) == NULL) {
		msg(m_alloc);
	} else {
		undoset();
		inserting = FALSE;
		(void) memcpy(scrap, egap, nscrap * sizeof (t_char));
		egap += nscrap;
		block();
		point = pos(egap);
		modified = TRUE;
	}
}

void
paste(void)
{
	if (nscrap <= 0) {
		msg(m_scrap);
	} else if (nscrap < egap-gap || growgap(nscrap)) {
		point = movegap(point);
		undoset();
		inserting = FALSE;
		memcpy(gap, scrap, nscrap * sizeof (t_char));
		gap += nscrap;
		point = pos(egap);
		modified = TRUE;
	}
}

void
version(void)
{
	msg(m_version);
}

void
prt_macros(void)
{
	t_keymap *kp;
	int used, total;
	unsigned char *ptr;

	erase();
	scrollok(stdscr, TRUE);
	for (used = total = 0, kp = key_map; kp->code != K_ERROR; ++kp) {
		if (kp->code == K_MACRO_DEFINE) {
			++total;
			if (kp->rhs != NULL) {
				++used;
				addch('{');
				ptr = (unsigned char *) kp->lhs;
				for (; *ptr != '\0'; ++ptr) {
					addstr(printable(*ptr));
				}
				addstr("}\t{");
				ptr = (unsigned char *) kp->rhs;
				for (; *ptr != '\0'; ++ptr) {
					addstr(printable(*ptr));
				}
				addstr("}\n");
				(void) more(used);
			}
		}
	}
	printw("\n%d/%d\n", used, total);
	scrollok(stdscr, FALSE);
	(void) more(-1);
	redraw();
}

void
macro(void)
{
	t_keymap *kp;
	size_t buflen, rhsoff;
	char *buf, *lhs, *rhs;

	if ((buf = (char *) malloc(BUFSIZ)) == NULL) {
		msg(m_alloc);
		return;
	}
	buf[0] = '\0';
	prompt(p_macro, buf, BUFSIZ);
	buflen = strlen(buf)+1;

	if ((lhs = strtok(buf, " \t")) == NULL) {
		prt_macros();
	} else if (buf < lhs) {
		/* Ideally we should shuffle the buffer down so that
		 * the lhs starts at the beginning of buffer.
		 */
		msg(m_error);
	} else if (encode(lhs) < 0) {
		msg(m_badescape);
	} else {
		kp = findkey(key_map, lhs);
		if ((rhs = strtok(NULL, " \t")) == NULL) {
			/* Delete macro. */
			if (kp == NULL || kp->code != K_MACRO_DEFINE) {
				msg(m_nomacro);
			} else {
				free(kp->lhs);
				kp->lhs = kp->rhs = NULL;
			}
		} else if (encode(rhs) < 0) {
			msg(m_badescape);
		} else {
			rhsoff = (size_t)(rhs - buf);
			if ((buf = (char *) realloc(buf, buflen)) == NULL) {
				free(buf);
				msg(m_alloc);
				return;
			}

			if (kp == NULL) {
				/* Find free slot to add macro. */
				for (kp = key_map; kp->code != K_ERROR; ++kp) {
					if (kp->code == K_MACRO_DEFINE
					&& kp->lhs == NULL)
						break;
				}
			}
			if (kp->code == K_ERROR) {
				msg(m_slots);
			} else if (kp->code == K_MACRO_DEFINE) {
				/* Change macro. */
				kp->lhs = buf;
				kp->rhs = buf + rhsoff;
				return;
			} else {
				msg(m_nomacro);
			}
		}
	}
	free(buf);
}

/*
 * Flip the case of a region.
 */
void
flipcase(void)
{
	t_char *p;
	t_region r;
	for (getregion(&r); r.left <= r.right; ++r.left) {
		p = ptr(r.left);
		if (islower(*p)) {
			*p = toupper(*p);
			modified = TRUE;
		} else if (isupper(*p)) {
			*p = tolower(*p);
			modified = TRUE;
		}
	}
	if (marker == NOMARK) {
		right();
	}
}

/*
 * Return 1 if offset points to first-half of CR-LF;
 * 2 if offset points to second-half of CR-LF; 0 otherwise.
 */
int
iscrlf(t_point offset)
{
	t_char *p;

	p = ptr(offset);
	if (*p == '\r') {
		/* Look to the right for '\n'. */
		if (++offset < pos(ebuf) && *ptr(offset) == '\n')
			return (1);
	} else if (*p == '\n') {
		/* Look to the left for '\r'. */
		if (pos(buf) < offset && *ptr(--offset) == '\r')
			return (2);
	}
	return (0);
}
