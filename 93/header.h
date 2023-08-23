/*
 * header.h
 *
 * Anthony's Editor July 93
 *
 * Copyright 1993, 1993 by Anthony Howe.  All rights reserved.  No warranty.
 */

#ifndef __header_h__
#define __header_h__	1

#ifdef __STDC__
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#endif /* __STDC__ */

#ifdef ATARI_ST
/* Atari's Sozobon C has ANSI-like libraries
 * and headers but no prototypes.
 */
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys\osbind.h>

#define	DRIVE_COLON	1
#define EITHER_SLASH	1

#ifndef FILE_MODE
#define FILE_MODE	0
#endif
#endif /* ATARI_ST */

#ifdef __WATCOMC__
#ifdef MSDOS
#define __MSDOS__	1
#endif
#endif /* __WATCOMC__ */

#ifdef __MSDOS__
#define DRIVE_COLON	1
#define EITHER_SLASH	1

#ifndef FILE_MODE
#define FILE_MODE	0
#endif
#endif /* __MSDOS__ */

#ifdef BSD
#ifndef __STDC__
#include <signal.h>
#include <varargs.h>
#include <strings.h>
extern char *getenv();
extern char *malloc();
extern char *realloc();
extern char *strtok();
#endif /* __STDC__ */
#endif /* BSD */

#ifndef SIG_ERR
#define	SIG_ERR		((void (*) _((int))) -1)
#endif

#include <assert.h>
#include <curses.h>
#include "key.h"

#ifdef BADCURSES
#define erasechar()	'\b'
#define killchar()	CTRL('x')
#define idlok(w,f)	OK
#endif /* BADCURSES */

#undef _
#ifdef __STDC__
#define _(x)		x
#else
#define _(x)		()
#define const
#endif

#ifndef TAG
#define TAG		"AE July 93"
#endif

#define VERSION		\
"0:" TAG "  Copyright 1993, 2023 by Anthony Howe.  No warranty."

#ifndef CONFIG
# ifdef __unix__
#  define CONFIG		".aerc"
# elif __MSDOS__
#  define CONFIG		"ae.rc"
# else
#  error "Add -DCONFIG='\"ae.comf\"' to makefile CFLAGS."
# endif
#endif /* CONFIG */

#ifndef CHUNK
#define CHUNK		8096L
#endif /* CHUNK */

#ifndef FILE_MODE
#define FILE_MODE	0600
#endif /* FILE_MODE */

/* Screen partitioning. */
#define MSGLINE		0
#define HELPLINE	1
#undef  TEXTLINE

#define TABWIDTH	8
#define TABSTOP(col)	(TABWIDTH - ((col) & (TABWIDTH-1)))

#define NOMARK		-1

typedef long SSIZE_T;			/* ssize_t not defined all systems. */
typedef char *t_msg;
typedef unsigned char t_char;
typedef long t_point;

typedef struct t_keytable {
	short key;
	void (*func) _((void));
	void (*disp) _((void));
} t_keytable;

typedef struct t_region {
	t_point left;
	t_point right;
} t_region;

/*
 * Function return codes.
 */
#define GETBLOCK_EOF		(-1L)
#define GETBLOCK_ERROR		(-2L)
#define GETBLOCK_ALLOC		(-3L)

/*
 * Some compilers define size_t as a unsigned 16 bit number while
 * t_point and off_t might be defined as a signed 32 bit number.
 * malloc(), realloc(), fread(), and fwrite() take size_t parameters,
 * which means there will be some size limits because size_t is too
 * small of a type.
 */
#define MAX_SIZE_T	((unsigned long) (size_t) ~0)
#define MAX_SSIZE_T	((unsigned long) (SSIZE_T) ~(size_t) 0 >> 1)

/*
 *
 */
extern int done;		/* Quit flag. */
extern int modified;		/* Text buffer modified flag. */
extern int modeless;		/* Command-set style. */
extern int msgflag;		/* True if msgline should be displayed. */
extern int inserting;

extern int cur_row;		/* Cursor screen row */
extern int cur_col;		/* Cursor screen column. */
extern int textline;		/* First screen line used for text. */

extern t_point point;		/* Cursor offset in text buffer. */
extern t_point pointline;	/* Cursor line number. */
extern t_point page;		/* Top of screen page. */
extern t_point epage;		/* End of screen page +1 */
extern t_point marker;		/* Block anchor point. */

extern t_char *buf;		/* Base of allocated text buffer. */
extern t_char *ebuf;		/* End of text buffer +1 */
extern t_char *gap;		/* Start of gap. */
extern t_char *egap;		/* End of gap +1 */

extern t_point nscrap;		/* Length of scrap buffer. */
extern t_char *scrap;		/* Allocated scrap buffer. */

extern int count;		/* Command repeat count. */
extern int input;		/* Current input character. */
extern char msgline[];		/* Message line input/output buffer. */
extern char filename[];		/* Current filename for text buffer. */
extern char temp[];		/* Temporary buffer. */
extern char *prog_name;		/* Name used to invoke editor. */

extern t_keytable table[];	/* Command jump table. */
extern t_keymap *key_map;	/* Command key mappings. */
extern t_keymap key_mode[];	/* Key mappings used in insert_mode() */
extern t_keymap key_vi[];	/* Default keys when no .aerc found. */

/* fatal() messages. */
extern t_msg f_ok;		/* EXIT_OK */
extern t_msg f_error;		/* EXIT_ERROR */
extern t_msg f_usage;		/* EXIT_USAGE */
extern t_msg f_initscr;		/* EXIT_FAILURE ... */
extern t_msg f_config;
extern t_msg f_alloc;

/* Messages. */
extern t_msg m_version;
extern t_msg m_help;
extern t_msg m_ok;
extern t_msg m_error;
extern t_msg m_alloc;
extern t_msg m_toobig;
extern t_msg m_scrap;
extern t_msg m_stat;
extern t_msg m_open;
extern t_msg m_close;
extern t_msg m_read;
extern t_msg m_write;
extern t_msg m_badname;
extern t_msg m_file;
extern t_msg m_modified;
extern t_msg m_saved;
extern t_msg m_loaded;
extern t_msg m_badescape;
extern t_msg m_nomacro;
extern t_msg m_slots;
extern t_msg m_interrupt;
extern t_msg m_eof;
extern t_msg m_undo;
extern t_msg m_no_match;
extern t_msg m_no_pattern;

/* Prompts */
extern t_msg p_macro;
extern t_msg p_notsaved;
extern t_msg p_press;
extern t_msg p_read;
extern t_msg p_write;
extern t_msg p_bwrite;
extern t_msg p_yes;
extern t_msg p_no;
extern t_msg p_quit;
extern t_msg p_more;
extern t_msg p_inc_search;
extern t_msg p_search;

extern t_msg message[];

extern void fatal _((t_msg));
extern void msg _((t_msg, ...));
extern char *getmsg _((t_msg));
extern char *strlwr _((char *));
extern char *strdup _((const char *));
extern char *strrep _((char *, int, int, int));
extern char *pathname _((char *, char *));
extern FILE *openrc _((char *));
extern long getblock _((FILE *, char **));
extern long encode _((char *));

extern void display _((void (*)(void)));
extern void dispfull _((void));
extern void dispcursor _((void));
extern t_point lnstart _((t_point));
extern t_point lnnext _((t_point));
extern t_point lncolumn _((t_point, int));
extern t_point segstart _((t_point, t_point));
extern t_point segnext _((t_point, t_point));
extern t_point upup _((t_point));
extern t_point dndn _((t_point));
extern void ruler _((int));
extern char *printable _((unsigned));

extern int growgap _((t_point));
extern t_point movegap _((t_point));
extern t_point pos _((t_char *));
extern t_char *ptr _((t_point));
extern void getregion _((t_region *));
extern int posix_file _((char *));
extern int save _((char *));
extern int load _((char *));
extern void undoset _((void));
extern void undosave _((void));
extern void undo _((void));

extern void backsp _((void));
extern void block _((void));
extern void bottom _((void));
extern void cut _((void));
extern void delete _((void));
extern void down _((void));
extern void help _((void));
extern void insert _((void));
extern void insert_mode _((void));
extern void left _((void));
extern void lnbegin _((void));
extern void lnend _((void));
extern void macro _((void));
extern void paste _((void));
extern void pgtop _((void));
extern void pgbottom _((void));
extern void pgmiddle _((void));
extern void pgdown _((void));
extern void pgup _((void));
extern void quit _((void));
extern void quit_ask _((void));
extern void redraw _((void));
extern void readfile _((void));
extern void right _((void));
extern void top _((void));
extern void up _((void));
extern void version _((void));
extern void wleft _((void));
extern void wright _((void));
extern void writefile _((void));
extern void flipcase _((void));
extern int iscrlf _((t_point));

extern void regex_search _((void));
extern void inc_search _((void));
extern void match_next _((void));
extern void match_prev _((void));

#endif /* __header_h__ */
