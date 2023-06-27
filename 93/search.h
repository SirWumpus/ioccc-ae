/*
 * search. (copied from LibSnert)
 *
 * Copyright 2015 by Anthony Howe. All rights reserved.
 */

#ifndef __search_h__
#define __search_h__ 1

typedef struct {
	const unsigned char *pattern;
	size_t length;
	size_t (*bm)[256];
	size_t (*qs)[256];
	unsigned max_err;
} Pattern;

extern void horspool_fini(Pattern *pp);
extern int horspool_init(Pattern *pp, const unsigned char *pattern, unsigned max_err);
extern long horspool_search(Pattern *pp, const unsigned char *str, size_t len);

extern void sunday_fini(Pattern *pp);
extern int sunday_init(Pattern *pp, const unsigned char *pattern, unsigned max_err);
extern long sunday_search(Pattern *pp, const unsigned char *str, size_t len);

extern void smith_fini(Pattern *pp);
extern int smith_init(Pattern *pp, const unsigned char *pattern, unsigned max_err);
extern long smith_search(Pattern *pp, const unsigned char *str, size_t len);

#endif /* __search_h__ */
