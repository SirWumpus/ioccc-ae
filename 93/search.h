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
	size_t qs[256];
} Pattern;

extern int sunday_init(Pattern *pp, const unsigned char *pattern);
extern long sunday_search(Pattern *pp, const unsigned char *str, size_t len);

#endif /* __search_h__ */
