/*
 * data.c
 *
 * Anthony's Editor July 93
 *
 * Copyright 1993, 2023 by Anthony Howe.  All rights reserved.  No warranty.
 */

#include "header.h"
#include "key.h"

int done;
int modified;
int modeless;

t_point point;
t_point page;
t_point epage;
t_point marker = NOMARK;

int cur_row, cur_col;
int textline = HELPLINE;

t_char *buf;
t_char *ebuf;
t_char *gap;
t_char *egap;

t_point nscrap;
t_char *scrap;

int input;
int msgflag;
char msgline[BUFSIZ];
char filename[BUFSIZ];
char temp[BUFSIZ];
char *prog_name;

t_keytable table[] = {
	{ K_CURSOR_LEFT, left, dispcursor },
	{ K_CURSOR_RIGHT, right, dispcursor },
	{ K_CURSOR_DOWN, down, dispcursor },
	{ K_CURSOR_UP, up, dispcursor },
	{ K_WORD_LEFT, wleft, dispcursor },
	{ K_WORD_RIGHT, wright, dispcursor },
	{ K_PAGE_UP, pgup, dispfull },
	{ K_PAGE_DOWN, pgdown, dispfull },
	{ K_LINE_LEFT, lnbegin, dispcursor },
	{ K_LINE_RIGHT, lnend, dispcursor },
	{ K_PAGE_TOP, pgtop, dispcursor },
	{ K_PAGE_BOTTOM, pgbottom, dispcursor },
	{ K_PAGE_MIDDLE, pgmiddle, dispcursor },
	{ K_FILE_TOP, top, dispfull },
	{ K_FILE_BOTTOM, bottom, dispfull },
	{ K_FLIP_CASE, flipcase, dispfull },
	{ K_DELETE_LEFT, backsp, dispfull },
	{ K_DELETE_RIGHT, del_right, dispfull },
	{ K_INSERT_ENTER, insert_mode, dispfull },
	{ K_LITERAL, insert, dispfull },
	{ K_BLOCK, block, dispfull },
	{ K_CUT, cut, dispfull },
	{ K_PASTE, paste, dispfull },
	{ K_UNDO, undo, dispfull },
	{ K_FILE_READ, readfile, dispfull },
	{ K_FILE_WRITE, writefile, dispfull },
	{ K_REDRAW, redraw, dispfull },
	{ K_HELP, help, NULL },
	{ K_QUIT, quit, NULL },
	{ K_QUIT_ASK, quit_ask, NULL },
	{ K_SHOW_VERSION, version, dispfull },
	{ K_MACRO, macro, dispfull },
	{ K_REGEX_SEARCH, regex_search, dispfull },
	{ K_INC_SEARCH, inc_search, dispfull },
	{ K_MATCH_NEXT, match_next, dispfull },
	{ K_MATCH_PREV, match_prev, dispfull },
	{ 0, NULL, dispfull }
};

t_keymap key_mode[] = {
	{ K_INSERT_EXIT, NULL },
	{ K_STTY_KILL, NULL },
	{ K_LITERAL, NULL },
	{ K_STTY_ERASE, "\b" },
	{ K_STTY_ERASE, "\x7f" },
	{ K_ERROR, NULL }
};

t_keymap key_vi[] = {
	{ K_CURSOR_LEFT,	"h" },
	{ K_CURSOR_RIGHT,	"l" },
	{ K_CURSOR_DOWN,	"j" },
	{ K_CURSOR_UP,		"k" },
	{ K_WORD_LEFT,		"b" },
	{ K_WORD_RIGHT,		"w" },
	{ K_PAGE_UP,		"\x02" },	/* ^B */
	{ K_PAGE_DOWN,		"\x06" },	/* ^F */
	{ K_LINE_LEFT,		"0" },
	{ K_LINE_RIGHT,		"$" },
	{ K_PAGE_TOP,		"H" },
	{ K_PAGE_BOTTOM,	"L" },
	{ K_PAGE_MIDDLE,	"M" },
	{ K_FILE_TOP,		"1G" },
	{ K_FILE_BOTTOM,	"G" },
	{ K_FLIP_CASE,		"~" },
	{ K_DELETE_LEFT,	"X" },
	{ K_DELETE_RIGHT,	"x" },
	{ K_BLOCK,		"\x1e" },	/* ^^ */
	{ K_CUT,		"v" },
	{ K_PASTE,		"P" },
	{ K_UNDO,		"u" },
	{ K_FILE_READ,		":r" },
	{ K_FILE_WRITE,		":w" },
	{ K_INSERT_ENTER,	"i" },
	{ K_INSERT_EXIT,	"\e" },		/* ^[ */
	{ K_REGEX_SEARCH,	"/" },
	{ K_INC_SEARCH,		"I" },
	{ K_MATCH_NEXT,		"n" },
	{ K_QUIT_ASK,		"q" },
	{ K_QUIT,		"Q" },
	{ K_LITERAL,		"\x16" },	/* ^V */
	{ K_REDRAW,		"\x12" },	/* ^R */
	{ K_HELP,		":h" },
	{ K_SHOW_VERSION,	":ver" },
	{ K_MACRO,		":map" },
	{ K_MACRO_DEFINE,	NULL },
	{ K_MACRO_DEFINE,	NULL },
	{ K_MACRO_DEFINE,	NULL },
	{ K_MACRO_DEFINE,	NULL },
	{ K_MACRO_DEFINE,	NULL },
	{ K_ERROR,		NULL }
};

t_msg m_help = "1:"
"Left, down, up, right	h  j  k  l	Insert on, off, erase	i  ESC  ERASE\n"
"Word left, right	b  w		Delete left, right	X  x\n"
"Page top, bottom, mid	H  L  M		Block, cut, paste, undo	^^  v  P  u\n"
"Page down, up		^F  ^B\t	Start, end of line	0  $\n"
"Top, bottom of file	1G  G		Invert case		~\n"
"Macros, redraw		:map  ^R	Literal escape		^V\n"
"Ere, inc. find, next	/  I  n		File read, write	:r  :w\n"
"Toggle help		:h		Version, quit		:ver  q  Q\n"
;

t_keymap *key_map;

t_msg m_version = VERSION;

t_msg f_ok = "2:%s: Terminated successfully.\n";
t_msg f_error = "3:%s: Unspecified error.\n";
t_msg f_usage = "4:usage: %s [-f <config>] [file]\n";
t_msg f_initscr = "5:%s: Failed to initialize the screen.\n";
t_msg f_config = "6:%s: Problem with configuration file.\n";
t_msg f_alloc = "7:%s: Failed to allocate required memory.\n";

t_msg m_ok = "8:Ok.";
t_msg m_error = "9:An error occured.";
t_msg m_alloc = "10:No more memory available.";
t_msg m_toobig = "11:File \"%s\" is too big to load.";
t_msg m_scrap = "12:Scrap is empty.  Nothing to paste.";
t_msg m_stat = "13:Failed to find file \"%s\".";
t_msg m_open = "14:Failed to open file \"%s\".";
t_msg m_close = "15:Failed to close file \"%s\".";
t_msg m_read = "16:Failed to read file \"%s\".";
t_msg m_write = "17:Failed to write file \"%s\".";
t_msg m_badname = "18:Not a portable POSIX file name.";
t_msg m_file = "19:\"%s\" %ldB %c";
t_msg m_saved = "20:\"%s\" %ldB saved.";
t_msg m_loaded = "21:\"%s\" %ldB read.";
/* t_msg m_modified = "22:File \"%s\" modified."; */
t_msg m_badescape = "23:Invalid control character or \\number not 0..255.";
t_msg m_nomacro = "24:No such macro defined.";
t_msg m_slots = "25:No more macro space.";
t_msg m_interrupt = "26:Interrupt.";
t_msg m_eof = "27:<< EOF >>";

t_msg p_macro = "28:Macro :";
t_msg p_notsaved = "29:File not saved.  Quit (y/n) ?";
t_msg p_press = "30:[ Press a key to continue. ]";
t_msg p_read = "31:Read file :";
t_msg p_write = "32:Write file :";
t_msg p_bwrite = "33:Write block :";
t_msg p_more = "34: more ";
t_msg p_yes = "35: y\b";
t_msg p_no = "36: n\b";
t_msg p_quit = "37: q\b";

/* t_msg m_undo = "38:Nothing to undo."; */

t_msg p_inc_search = "39:Incremental :";
t_msg m_no_match = "40:No match.";
t_msg p_search = "41:ERE pattern:";
t_msg m_no_pattern = "42:No search pattern.";

t_msg message[43];

/* end */
