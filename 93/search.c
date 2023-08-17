/*
 * search.c
 *
 * Copyright 2015 by Anthony Howe. All rights reserved.
 */

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "search.h"

#define min(a, b)	(a < b ? a : b)
#define max(a, b)	(a < b ? b : a)

#define INFO(...)       { if (0 < debug) { warnx(__VA_ARGS__); } }
#define DEBUG(...)      { if (1 < debug) { warnx(__VA_ARGS__); } }
#define DUMP(...)       { if (2 < debug) { warnx(__VA_ARGS__); } }

static int debug;

/*
 * Boyer-Moore-Horspool-Sunday search algorithm (quick search variant).
 *
 * https://csclub.uwaterloo.ca/~pbarfuss/p132-sunday.pdf
 * http://www-igm.univ-mlv.fr/~lecroq/string/node19.html#SECTION00190
 * http://alg.csie.ncnu.edu.tw/course/StringMatching/Quick%20Searching.ppt
 */
int
sunday_init(Pattern *pp, const unsigned char *pattern)
{
	int i;

	pp->pattern = pattern;
	pp->length = strlen((char *)pattern);

	for (i = 0; i < (sizeof (pp->qs) / sizeof (*pp->qs)); i++) {
		pp->qs[i] = pp->length + 1;
	}
	for (i = 0; i < pp->length; i++) {
		pp->qs[pattern[i]] = pp->length - i;
		DEBUG("ch='%c' delta=%lu", pattern[i], pp->qs[pattern[i]]);
	}
	DEBUG("ch=* delta=%lu", pp->qs[0]);

	return 0;
}

long
sunday_search(Pattern *pp, const unsigned char *str, size_t len)
{
	long offset = 0;

	/* Note that this can reference the NUL byte when "offset
	 * + pp->length == len", which is not an index bounds error
	 * in C, but when ported to other languages like Java or C#
	 * that have no sentinel end of string byte, this has to be
	 * handled specially.
	 */
	while (offset + pp->length <= len) {
		INFO("off=%ld str=\"%s\" delta=%lu", offset, str+offset, pp->qs[str[offset + pp->length]]);
		if (memcmp(str + offset, pp->pattern, pp->length) == 0) {
			INFO("return offset=%ld", offset);
			return offset;
		}
		offset += pp->qs[str[offset + pp->length]];
	}

	INFO("return -1 no match");
	return -1;
}

#ifdef TEST

#include <errno.h>
#include <stdio.h>
#include <getopt.h>

#define LINE_SIZE	2048

static const char usage[] =
"usage: search [-bv] string file ...\n"
"-b\t\tbracket the first match\n"
"-v\t\tverbose debug\n"
;

size_t
inputline(FILE *fp, unsigned char *buf, size_t size)
{
	int ch;
	size_t len;

	if (size == 0)
		return 0;
	size--;

	for (len = 0; len < size; ) {
		if ((ch = fgetc(fp)) == EOF)
			break;
		buf[len++] = ch;
		if (ch == '\n')
			break;
	}

	buf[len] = '\0';

	return len;
}

int
main(int argc, char **argv)
{
	FILE *fp;
	Pattern pat;
	size_t line_len;
	long lineno, offset;
	int brackets, ch, rc, argi;
	unsigned char line[LINE_SIZE];

	brackets = 0;

	while ((ch = getopt(argc, argv, "bk:v")) != -1) {
		switch (ch) {
		case 'b':
			brackets = 1;
			break;
		case 'k':
			break;
		case 'v':
			debug++;
			break;
		default:
			optind = argc;
		}
	}

	if (argc <= optind + 1) {
		(void) fputs(usage, stderr);
		return 2;
	}

	rc = 1;
	sunday_init(&pat, (const unsigned char *)argv[optind]);

	for (argi = optind+1; argi < argc; argi++) {
		if ((fp = fopen(argv[argi], "r")) == NULL) {
			(void) fprintf(stderr, "%s: %s (%d)\n", argv[argi], strerror(errno), errno);
			continue;
		}

		lineno = 0;
		while (0 < (line_len = inputline(fp, line, sizeof (line)))) {
			lineno++;
			if ((offset = sunday_search(&pat, line, line_len)) != -1) {
				if (optind+2 < argc) {
					(void) printf("%s: ", argv[argi]);
				}
				if (brackets) {
					(void) printf(
						"%ld %ld %.*s[%.*s]%s",
						lineno, offset,
						(int) offset, line,
						(int) pat.length, line + offset,
						line + offset + pat.length
					);
				} else {
					(void) printf("%ld %-2ld %s", lineno, offset, line);
				}
				rc = 0;
			}
		}

		(void) fclose(fp);
	}

	return rc;
}

#endif
