/*
 * command.c
 *
 * Anthony's Editor July 93
 *
 * Copyright 1993, 2023 by Anthony Howe.  All rights reserved.  No warranty.
 */

#include <ctype.h>
#include <regex.h>
#include "header.h"
#include "search.h"

static int editing;
static regex_t ere;
static int ere_ready;
static int ere_anchor_only;
static Pattern pat;
static t_point start_point = NOMARK;		/* Point at start a search. */


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
	promptmsg(getmsg(p_more));
	ch = getliteral();
	return (ch != getmsg(p_quit)[1] && ch != getmsg(p_no)[1]);
}

void
top(void)
{
	editing = FALSE;
	point = 0;
}

void
bottom(void)
{
	editing = FALSE;
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
	int col;
#pragma GCC diagnostic pop

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
	editing = FALSE;
	if (0 < point && iscrlf(--point) == 2) {
		--point;
	}
}

void
right(void)
{
	editing = FALSE;
	if (point < pos(ebuf) && iscrlf(point++) == 1) {
		++point;
	}
}

void
up(void)
{
	editing = FALSE;
	point = lncolumn(upup(point), cur_col);
	if (iscrlf(point) == 2) {
		--point;
	}
}

void
down(void)
{
	editing = FALSE;
	point = lncolumn(dndn(point), cur_col);
	if (iscrlf(point) == 2) {
		--point;
	}
}

void
lnbegin(void)
{
	editing = FALSE;
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
	editing = FALSE;
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
	editing = FALSE;
	t_point epoint = pos(ebuf);
	while (isalnum(*ptr(point)) && point < epoint) {
		++point;
	}
	while (!isalnum(*ptr(point)) && point < epoint) {
		++point;
	}
}

void
pgtop(void)
{
	editing = FALSE;
	point = page;
}

void
pgbottom(void)
{
	editing = FALSE;
	point = epage-1;
	lnbegin();
}

void
pgmiddle(void)
{
	int middle = (LINES - textline) / 2 + textline;
	editing = FALSE;
	point = page;
	for (cur_row = textline; cur_row < middle; cur_row++) {
		down();
	}
}

void
pgdown(void)
{
	editing = FALSE;
	page = point = upup(epage);
	while (textline < cur_row--) {
		down();
	}
	epage = pos(ebuf);
}

void
pgup(void)
{
	int i = LINES;
	editing = FALSE;
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
	if (!editing){
		undoset();
		editing = TRUE;
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
		if (!editing){
			undoset();
			editing = TRUE;
		}
		if (*--gap == '\n' && buf < gap && gap[-1] == '\r') {
			--gap;
		}
		point = pos(egap);
		modified = TRUE;
	}
}

void
del_right(void)
{
	if (marker != NOMARK && point < marker) {
		cut();
		return;
	}
	point = movegap(point);
	if (egap < ebuf) {
		if (!editing){
			undoset();
			editing = TRUE;
		}
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
		editing = FALSE;
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
	free(scrap);
	scrap = NULL;
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
		editing = FALSE;
		(void) memcpy(scrap, egap, nscrap * sizeof (t_char));
		egap += nscrap;
		point = pos(egap);
		modified = TRUE;
		marker = NOMARK;
	}
}

void
paste(void)
{
	if (nscrap <= 0) {
		msg(m_scrap);
		beep();
	} else if (nscrap < egap-gap || growgap(nscrap)) {
		point = movegap(point);
		undoset();
		editing = FALSE;
		(void) memcpy(gap, scrap, nscrap * sizeof (t_char));
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

void
match_next(void)
{
	t_char *p;
	off_t offset;
	size_t length;
	static size_t prev_match_length = 0;

	editing = FALSE;

	/* Current cursor point. */
	p = ptr(point);
	if (p == ebuf) {
		/* Wrap around from end of buffer. */
		point = 0;
		p = buf;
	} else if (prev_match_length == 0 && (ere_anchor_only == '$' || (point == 0 && ere_anchor_only == '^'))) {
		/* Allows /^/ or /$/ to advance to start or end of next
		 * line respectively.  The RE /^$/, ^pat/, /pat$/, and
		 * /^pat$/ are fine.
		 *
		 * Note special cases:
		 *
		 * - CRLF newlines
		 *
		 *	Pattern /^/
		 *	-----------
		 *      BOFtext\r\n\r\nterminated\r\nEOF
		 *         ^       ^   ^             ^
		 *      BOFtext\r\n\r\nnot terminatedEOF
		 *         ^       ^   ^
		 *
		 *	Pattern /$/
		 *	-----------
		 *	BOFtext\r\n\r\nterminated\r\nEOF
		 *               ^   ^             ^ ^
		 *	BOFtext\r\n\r\nnot terminatedEOF
		 *               ^   ^               ^
		 *
		 * - LF newlines
		 *
		 *	Pattern /^/
		 *	-----------
		 *      BOFtext\n\nterminated\nEOF
		 *         ^     ^ ^           ^
		 *      BOFtext\n\nnot terminatedEOF
		 *         ^     ^ ^
		 *
		 *	Pattern /$/
		 *	-----------
		 *	BOFtext\n\nterminated\nEOF
		 *             ^ ^           ^ ^
		 *	BOFtext\n\nnot terminatedEOF
		 *             ^ ^               ^
		 */
		p = ptr(++point);
	}

	/* Find next match. */
	if (ere_ready) {
		regmatch_t matches[1];
		/* REG_NOTBOL allows /^/ to advane to start of next line. */
		if (regexec(&ere, (char *)p, 1, matches, point == 0 ? 0 : REG_NOTBOL) == REG_NOMATCH) {
			offset = -1;
		} else {
			offset = matches[0].rm_so;
			length = matches[0].rm_eo - offset;
			prev_match_length = length;
		}
	} else if (0 < pat.length) {
		offset = sunday_search(&pat, (char *)p, egap == ebuf ? gap-p : ebuf-p);
		length = pat.length;
	} else {
		start_point = NOMARK;
		msg(m_no_pattern);
		beep();
		return;
	}

	if (0 <= offset) {
		/* Highlight text from marker to end of match. */
		marker = point + offset;
		point = marker + length;
	} else if (point <= start_point || marker == NOMARK) {
		/* Match not found. */
		prev_match_length = ~0;
		start_point = NOMARK;
		marker = NOMARK;
		msg(m_no_match);
		beep();
	} else {
		/* Wrap search to the start of the buffer.  Clear marker
		 * to avoid multiple wrap-arounds (and seg.fault).  If
		 * the wrap-around sets marker again then a match was
		 * found, possibly a previous match.
		 */
		prev_match_length = ~0;
		marker = NOMARK;
		point = 0;
		match_next();
	}
}

void
match_prev(void)
{
}

void
inc_search(void)
{
	t_fld fld;

	regfree(&ere);
	ere_ready = FALSE;
	promptmsg(p_inc_search);
	getyx(stdscr, fld.row, fld.col);

	temp[0] = '\0';
	fld.index = 0;
	fld.buffer = temp;
	fld.length = COLS - fld.col;

	/* If the gap is at the start or end of file, then it is
	 * already out of the way.  No need to move anything.
	 */
	if (buf < gap && egap < ebuf) {
		/* Move the gap out of the way of forward searching. */
		point = movegap(point);
	}
	marker = point;

	/* Where the search started, in case we wrap aorund. */
	start_point = point;

	for (;;) {
		if (!getnext(&fld)) {
			break;
		}

		/* Updated pattern with next input key. */
		sunday_init(&pat, fld.buffer);

		match_next();
		display(dispfull);
		if (marker != NOMARK) {
			point = marker;
		}

		/* Redraw prompt and current search input.*/
		promptmsg(p_inc_search);
		addstr(temp);
		getyx(stdscr, fld.row, fld.col);
		refresh();
	}
}

void
regex_search(void)
{
	regfree(&ere);
	ere_ready = FALSE;

	for (temp[0] = '\0'; ; ) {
		prompt(p_search, temp, BUFSIZ);
		if (*temp == '\0') {
			break;
		}
		if (regcomp(&ere, temp, REG_EXTENDED|REG_NEWLINE) == 0) {
			if (temp[1] == '\0') {
				/* Single character pattern, eg. /^/ /$/ /./ */
				ere_anchor_only = temp[0];
			} else {
				ere_anchor_only = -1;
			}

			/* If the gap is at the start or end of file, then it is
			 * already out of the way.  No need to move anything.
			 */
			if (buf < gap && egap < ebuf) {
				/* Move the gap out of the way of forward searching. */
				point = movegap(point);
			}
			marker = point;

			/* Where the search started, in case we wrap aorund. */
			start_point = point;

			ere_ready = TRUE;
			match_next();
			break;
		}
		beep();
	}
}
