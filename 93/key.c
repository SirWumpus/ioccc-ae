/*
 * key.c
 *
 * Anthony's Editor July 93
 *
 * Public Domain 1991, 1993 by Anthony Howe.  No warranty.
 */

#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include "header.h"
#include "key.h"

/* Variable length structure. */
typedef struct t_input {
	struct t_input *next;
	char *ptr;
	char buf[1];
} t_input;

static t_input *istack = NULL;
static char blank[] = " \t\r\n";

static int k_default _((t_keymap *));
static int k_define _((t_keymap *));
static int k_erase  _((t_keymap *));
static int k_itself _((t_keymap *));
static int k_kill _((t_keymap *));
static int k_token _((t_keymap *));
static t_keymap *growkey _((t_keymap *, size_t));
static int ipush _((char *));
static int ipop _((void));
static void iflush _((void));

t_keyinit keywords[] = {
	{ K_INSERT_ENTER, ".insert_enter", k_default },
	{ K_INSERT_EXIT, ".insert_exit", k_default },
	{ K_DELETE_LEFT, ".delete_left", k_default },
	{ K_DELETE_RIGHT, ".delete_right", k_default },
	{ K_FLIP_CASE, ".flip_case", k_default },
	{ K_BLOCK, ".block", k_default },
	{ K_CUT, ".cut", k_default },
	{ K_PASTE, ".paste", k_default },
	{ K_UNDO, ".undo", k_default },
	{ K_CURSOR_UP, ".cursor_up", k_default },
	{ K_CURSOR_DOWN, ".cursor_down", k_default },
	{ K_CURSOR_LEFT, ".cursor_left", k_default },
	{ K_CURSOR_RIGHT, ".cursor_right", k_default },
	{ K_PAGE_UP, ".page_up", k_default },
	{ K_PAGE_DOWN, ".page_down", k_default },
	{ K_WORD_LEFT, ".word_left", k_default },
	{ K_WORD_RIGHT, ".word_right", k_default },
	{ K_LINE_LEFT, ".line_left", k_default },
	{ K_LINE_RIGHT, ".line_right", k_default },
	{ K_PAGE_TOP, ".page_top", k_default },
	{ K_PAGE_BOTTOM, ".page_bottom", k_default },
	{ K_FILE_TOP, ".file_top", k_default },
	{ K_FILE_BOTTOM, ".file_bottom", k_default },
	{ K_HELP, ".help", k_default },
	{ K_HELP_OFF, ".help_off", k_token },
	{ K_MACRO, ".macro", k_default },
	{ K_MACRO_DEFINE, ".macro_define", k_define },
	{ K_QUIT, ".quit", k_default },
	{ K_QUIT_ASK, ".quit_ask", k_default },
	{ K_FILE_READ, ".file_read", k_default },
	{ K_FILE_WRITE, ".file_write", k_default },
	{ K_STTY_ERASE, ".stty_erase", k_erase },
	{ K_STTY_KILL, ".stty_kill", k_kill },
	{ K_ITSELF, ".itself", k_itself },
	{ K_REDRAW, ".redraw", k_default },
	{ K_SHOW_VERSION, ".show_version", k_default },
	{ K_LITERAL, ".literal", k_default },
	{ K_INC_SEARCH, ".inc_search", k_default },
	{ K_INC_NEXT, ".inc_next", k_default },
	{ K_INC_PREV, ".inc_prev", k_default },
	{ K_ERROR, NULL, NULL }
};

int
ismsg(str)
char *str;
{
	char *ptr;
	for (ptr = str; isdigit(*ptr); ++ptr)
		;
	return (str < ptr && *ptr == ':');
}

/*
 * Read a configuration file from either the current directory or
 * the user's home directory.  Return an error status.  Pass-back
 * either a pointer to a key mapping table, or NULL if an error
 * occured.
 */
int
initkey(fn, keys)
char *fn;
t_keymap **keys;
{
	FILE *fp;
	int error;
	t_keyinit *kp;
	t_keymap *array;
	size_t len, count;
	char *buf, *token, *lhs, *rhs;

	*keys = NULL;
	if ((fp = openrc(fn)) == NULL)
		return (INITKEY_OPEN);

	/* Allocate an array big enough to hold at least one of everything. */
	if ((array = growkey(NULL, len = K_MAX_CODES)) == NULL) {
		error = INITKEY_ALLOC;
		goto error1;
	}

	count = 0;
	while ((error = getblock(fp, &buf)) != GETBLOCK_EOF) {
		if (error == GETBLOCK_ALLOC) {
			error = INITKEY_ALLOC;
			goto error1;
		}

		/* Strip \r\n from end of buffer. */
		if ((token = strrchr(buf, '\n')) != NULL) {
			if (buf < token && token[-1] == '\r')
				--token;
			*token = '\0';
		}

		if (ismsg(buf)) {
			long index = strtol(buf, &token, 0);
			(void) encode(token);
			if (0 < index)
				message[index] = token+1;
			continue;
		}

		if (buf[0] != '.'
		|| (token = strtok(buf, blank)) == NULL
		|| (kp = findikey(keywords, strlwr(token))) == NULL) {
			free(buf);
			continue;
		}

		array[count].code = kp->code;
		/* Determine lhs and rhs parameters. */
		if ((token = strtok(NULL, "")) != NULL) {
			/* Find start of parameters. */
			token += strspn(token, blank);
			/* Shuffle parameters to the start of the buffer. */
			(void) strcpy(buf, token);
			/* Assume shrinking buffer succeeds. */
			buf = (char *) realloc(buf, strlen(buf) + 1);
			/* Parse lhs, which should equal start of buffer. */
			array[count].lhs = token = strtok(buf, blank);
			if (encode(token) <= 0) {
				error = INITKEY_ERROR;
				goto error1;
			}
			/* Find rhs if present. */
			if ((token = strtok(NULL, blank)) != NULL
			&& encode(token) <= 0) {
				error = INITKEY_ERROR;
				goto error1;
			}
			array[count].rhs = token;
		} else {
			/* No parameters for keyword. */
			array[count].lhs = array[count].rhs = NULL;
			free(buf);
		}

		if (kp->fn != NULL && !(*kp->fn)(&array[count])) {
			error = INITKEY_ERROR;
			goto error1;
		}
		++count;

		if (len <= count) {
			t_keymap *new;
			len += K_MAX_CODES;
			if ((new = growkey(array, len)) == NULL) {
				error = INITKEY_ALLOC;
				goto error1;
			}
			array = new;
		}
	}
	error = INITKEY_OK;
	*keys = array;
error1:
	(void) fclose(fp);
	array[count].code = K_ERROR;
	if (error != INITKEY_OK)
		finikey(array);
	return (error);
}

void
finikey(keys)
t_keymap *keys;
{
	t_keymap *kp;
	if (keys != NULL) {
		for (kp = keys; kp->code != K_ERROR; ++kp) {
			if (kp->lhs != NULL)
				free(kp->lhs);
		}
		free(keys);
	}
}

/*
 * .function string
 */
static int
k_default(kp)
t_keymap *kp;
{
	if (kp->lhs == NULL)
		return (FALSE);
	if (kp->rhs != NULL) {
		free(kp->lhs);
		return (FALSE);
	}
	return (TRUE);
}

/*
 * .macro_define
 * .macro_define lhs rhs
 *
 * The first case is used as a place holder to reserve macro
 * space.  The second case actual defines a macro.
 */
static int
k_define(kp)
t_keymap *kp;
{
	if (kp->lhs != NULL && kp->rhs == NULL) {
		free(kp->lhs);
		return (FALSE);
	}
	return (TRUE);
}

/*
 * .token
 */
static int
k_token(kp)
t_keymap *kp;
{
	if (kp->lhs != NULL) {
		free(kp->lhs);
		return (FALSE);
	}
	return (TRUE);
}

/*
 * .itself character
 */
static int
k_itself(kp)
t_keymap *kp;
{
	if (!k_default(kp))
		return (FALSE);
	kp->code = *(unsigned char *) kp->lhs;
	kp->lhs[1] = '\0';
	return (TRUE);
}

/*
 * .stty_erase
 */
static int
k_erase(kp)
t_keymap *kp;
{
	char buf[2];

	if (!k_token(kp))
		return (FALSE);
	buf[0] = erasechar();
	buf[1] = '\0';
	return ((kp->lhs = strdup(buf)) != NULL);
}

/*
 * .stty_kill
 */
static int
k_kill(kp)
t_keymap *kp;
{
	char buf[2];

	if (!k_token(kp))
		return (FALSE);
	buf[0] = killchar();
	buf[1] = '\0';
	return ((kp->lhs = strdup(buf)) != NULL);
}

/*
 * Find token and return corresponding table entry; else NULL.
 */
t_keymap *
findkey(kp, token)
t_keymap *kp;
char *token;
{
	for (; kp->code != K_ERROR; ++kp)
		if (kp->lhs != NULL && strcmp(token, kp->lhs) == 0)
			return (kp);
	return (NULL);
}

t_keyinit *
findikey(kp, token)
t_keyinit *kp;
char *token;
{
	for (; kp->code != K_ERROR; ++kp)
		if (kp->lhs != NULL && strcmp(token, kp->lhs) == 0)
			return (kp);
	return (NULL);
}

/*
 *
 */
static t_keymap *
growkey(array, len)
t_keymap *array;
size_t len;
{
	t_keymap *new;
	if (len == 0)
		return (NULL);
	len *= sizeof (t_keymap);
	if (array == NULL)
		return ((t_keymap *) malloc(len));
	return ((t_keymap *) realloc(array, len));
}

int
getkey(keys)
t_keymap *keys;
{
	t_keymap *k;
	int submatch;
	static char buffer[K_BUFFER_LENGTH];
	static char *record = buffer;

	/* If recorded bytes remain, return next recorded byte. */
	if (*record != '\0')
		return (*(unsigned char *)record++);
	/* Reset record buffer. */
	record = buffer;
	do {
		if (K_BUFFER_LENGTH < record - buffer) {
			record = buffer;
			buffer[0] = '\0';
			return (K_ERROR);
		}
		/* Read and record one byte. */
		*record++ = getliteral();
		*record = '\0';

		/* If recorded bytes match any multi-byte sequence... */
		for (k = keys, submatch = FALSE; k->code != K_ERROR; ++k) {
			if (k->lhs == NULL || k->code == K_DISABLED)
				continue;
			if (strncmp(buffer, k->lhs, record-buffer) != 0)
				continue;
			if (k->lhs[record-buffer] == '\0') {
				/* Exact match. */
				if (k->code != K_MACRO_DEFINE) {
					/* Return extended key code. */
					return (k->code);
				}
				if (k->rhs != NULL) {
					(void) ipush(k->rhs);
					record = buffer;
				}
			}
			/* Recorded bytes match anchored substring. */
			submatch = TRUE;
			break;
		}
		/* If recorded bytes matched an anchored substring, loop. */
	} while (submatch);
	/* Return first recorded byte. */
	record = buffer;
	return (*(unsigned char *)record++);
}

int
getliteral()
{
	int ch;

	ch = ipop();
	if (ch == EOF)
		return ((unsigned) getch());
	return (ch);
}

/*
 * Return true if a new input string was pushed onto the stack,
 * else false if there was no more memory for the stack.
 */
static int
ipush(buf)
char *buf;
{
	t_input *new;

	new = (t_input *) malloc(sizeof (t_input) + strlen (buf));
	if (new == NULL)
		return (FALSE);
	(void) strcpy(new->buf, buf);
	new->ptr = new->buf;
	new->next = istack;
	istack = new;
	return (TRUE);
}

/*
 * Pop and return a character off the input stack.  Return EOF if
 * the stack is empty.  If the end of an input string is reached,
 * then free the node.  This will allow clean tail recursion that
 * won't blow the stack.
 */
static int
ipop()
{
	int ch;
	t_input *node;

	if (istack == NULL)
		return (EOF);
	ch = (unsigned) *istack->ptr++;
	if (*istack->ptr == '\0') {
		node = istack;
		istack = istack->next;
		free(node);
	}
	return (ch);
}

/*
 * Flush the entire input stack.
 */
static void
iflush()
{
	t_input *node;

	while (istack != NULL) {
		node = istack;
		istack = istack->next;
		free(node);
	}
}

int
ismacro()
{
	return (istack != NULL);
}

/*
 * Field input.
 */

typedef struct t_keyfield {
	int code;
	int (*func) _((t_fld *));
} t_keyfield;

static int
fld_done(t_fld *ignore)
{
	return (FALSE);
}

static int
fld_left(t_fld *fld)
{
	int row, col, max_row, max_col;
	getyx(stdscr, row, col);
	getmaxyx(stdscr, max_row, max_col);
	if (0 < fld->index) {
		--fld->index;
		/* Assume that if 0 < fld->index then fld->row <= row
		 * and fld->col < col.  So when fld->index == 0, then
		 * fld->row == row and fld->col == col.
		 */
		if (0 < col) {
			--col;
		} else if (0 < row) {
			/* Handle reverse line wrap. */
			--row;
			col = max_col-1;
		}
		move(row, col);
	}
	return (TRUE);
}

static int
fld_erase(t_fld *fld)
{
	int row, col;
	if (0 < fld->index) {
		fld_left(fld);
		getyx(stdscr, row, col);
		addch(' ');
		move(row, col);
		fld->buffer[fld->index] = '\0';
	}
	return (TRUE);
}

static int
fld_kill(t_fld *fld)
{
	move(fld->row, fld->col);
	while (0 < fld->index--) {
		addch(' ');
	}
	move(fld->row, fld->col);
	fld->buffer[0] = '\0';
	fld->index = 0;
	return (TRUE);
}

static int
fld_append(t_fld *fld)
{
	if (fld->index < fld->length) {
		if (!ISFUNCKEY(fld->key)) {
			fld->buffer[fld->index++] = fld->key;
			fld->buffer[fld->index] = '\0';
			addch(fld->key);
		}
	}
	return (fld->index < fld->length);
}

#define ERASE_KEY	0
#define KILL_KEY	1

static t_keyfield ktable[] = {
	{ K_STTY_ERASE, fld_erase },
	{ K_STTY_KILL, fld_kill },
	{ '\r', fld_done },
	{ '\n', fld_done },
	{ '\e', fld_done },
	{ '\b', fld_erase },
	{ -1, fld_append }
};

#ifndef getmaxyx
#define getmaxyx(w,r,c)		(r=LINES,c=COLS)
#endif

int
getnext(t_fld *fld)
{
	t_keyfield *k;

	fld->key = getliteral();

	/* Lookup which action to take. */
	for (k = ktable; k->code != -1 && k->code != fld->key; ++k) {
		;
	}

	/* The input buffer may already have a value, eg. the
	 * current filename.  We can keep it by typing enter
	 * or replace it by typing a character.
	 */
	if (fld->index <= 0 && k->func == fld_append) {
		fld_kill(fld);
	}

	if (k->func != NULL && !(*k->func)(fld)) {
		/* End input. */
		fld->buffer[fld->index] = '\0';
		return FALSE;
	}

	/* Continue input. */
	return TRUE;
}

int
getinput(char *buf, size_t size)
{
	t_fld fld;

	fld.buffer = buf;
	fld.index = strlen(fld.buffer);

	/* promptmsg() will have already positioned the cursor. */
	getyx(stdscr, fld.row, fld.col);
	fld.length = COLS - fld.col - 1;

	/* Enough functional field input space? */
	if (fld.length <= fld.index || fld.length < MIN_FIELD_WIDTH || size <= fld.length) {
		return FALSE;
	}

	ktable[ERASE_KEY].code = erasechar();
	ktable[KILL_KEY].code = killchar();

	addstr(fld.buffer);
	move(fld.row, fld.col);

	do {
		refresh();
	} while (getnext(&fld));

	return (TRUE);
}
