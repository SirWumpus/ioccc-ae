/*
 * key.h
 *
 * Anthony's Editor July 93
 *
 * Copyright 1993, 1993 by Anthony Howe.  All rights reserved.  No warranty.
 */

#ifndef __key_h__
#define __key_h__	1

#undef _
#ifdef __STDC__
#define _(x)	x
#else
#define _(x)	()
#endif

#define K_BUFFER_LENGTH		256
#define ISFUNCKEY(x)		((x) < 0)

/*
 * Command key constants.
 */
#define K_ERROR		(-1)
#define K_DISABLED	(-2)

/* Edit functions. */
#define K_INSERT_ENTER	(-101)
#define K_INSERT_EXIT	(-102)
#define K_DELETE_LEFT	(-103)
#define K_DELETE_RIGHT	(-104)
#define K_FLIP_CASE	(-105)
#define K_BLOCK		(-106)
#define K_CUT		(-107)
#define K_PASTE		(-108)
#define K_UNDO		(-109)

/* Cursor motion. */
#define K_CURSOR_UP	(-110)
#define K_CURSOR_DOWN	(-111)
#define K_CURSOR_LEFT	(-112)
#define K_CURSOR_RIGHT	(-113)
#define K_PAGE_UP	(-114)
#define K_PAGE_DOWN	(-115)
#define K_WORD_LEFT	(-116)
#define K_WORD_RIGHT	(-117)
#define K_LINE_LEFT	(-118)
#define K_LINE_RIGHT	(-119)
#define K_FILE_TOP	(-120)
#define K_FILE_BOTTOM	(-121)

/* Support functions. */
#define K_ITSELF	(-122)
#define K_REDRAW	(-123)
#define K_SHOW_VERSION	(-124)
#define K_HELP		(-125)
#define K_HELP_OFF	(-126)
#define K_HELP_TEXT	(-127)
#define K_MACRO		(-128)
#define K_MACRO_DEFINE	(-129)
#define K_LITERAL	(-130)
#define K_QUIT		(-131)
#define K_QUIT_ASK	(-132)
#define K_FILE_READ	(-133)
#define K_FILE_WRITE	(-134)
#define K_STTY_ERASE	(-135)
#define K_STTY_KILL	(-136)
#define K_COUNT		(-137)

#define K_MAX_CODES	38

/* 
 * Function error codes. 
 */
#define INITKEY_OK	0
#define INITKEY_OPEN	1
#define INITKEY_ALLOC	2
#define INITKEY_ERROR	3

/*
 * ASCII Control Codes 
 */
#undef CTRL
#define CTRL(x)		((x) & 0x1f)

typedef struct t_keymap {
	short code;		/* Function code. */
	char *lhs;		/* Left hand side invokes function or macro. */
	char *rhs;		/* Right hand side macro expansion. */
} t_keymap;

typedef struct t_keyinit {
	short code;
	char *lhs;
	int (*fn) _((t_keymap *));
} t_keyinit;

typedef struct t_fld {
	int row;
	int col;
	int key;
	size_t index;
	size_t length;
	char *buffer;
} t_fld;

#define MIN_FIELD_WIDTH 10

extern int initkey _((char *, t_keymap **));
extern void finikey _((t_keymap *));
extern t_keymap *findkey _((t_keymap *, char *));
extern t_keyinit *findikey _((t_keyinit *, char *));
extern int getliteral _((void));
extern int getkey _((t_keymap *));
extern int getinput _((char *, size_t));
extern int getnext _((t_fld *));
extern int ismacro _((void));

#endif /* __key_h__ */
