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
 * Boyer-Moore-Horspool search algorithm.
 *
 * https://en.wikipedia.org/wiki/Boyer%E2%80%93Moore%E2%80%93Horspool_algorithm
 * http://www-igm.univ-mlv.fr/~lecroq/string/node18.html#SECTION00180
 * http://alg.csie.ncnu.edu.tw/course/StringMatching/Horspool.ppt
 *
 * https://www.cs.hut.fi/u/tarhio/papers/abm.pdf
 *	Approximate Boyer-Moore String Matching
 *	Jorma Tarhio And  Esko Ukkonen, 1989
 *
 * http://t2.ecp168.net/webs@73/cyberhood/Approximate_String_Matching/BHM_approximate_string_Algorithm.ppt
 */
int
horspool_init(Pattern *pp, const unsigned char *pattern, unsigned max_err)
{
	long i, k, m;

	pp->max_err = max_err;
	pp->pattern = pattern;
	pp->length = strlen((char *)pattern);
	m = pp->length - 1;

	if (pp->length <= max_err) {
		errno = EINVAL;
		return -1;
	}

	if ((pp->bm = malloc((max_err+1) * sizeof (*pp->bm))) == NULL) {
		return -1;
	}

	for (k = 0; k <= max_err; k++) {
		for (i = 0; i < (sizeof (*pp->bm) / sizeof (**pp->bm)); i++) {
			pp->bm[k][i] = pp->length;
		}
		pp->bm[k][pattern[m - k]] = pp->length - k;
		for (i = 0; i < m - k; i++) {
			pp->bm[k][pattern[i]] = m - k - i;
		}
	}

	return 0;
}

void
horspool_fini(Pattern *pp)
{
	if (pp != NULL && pp->bm != NULL) {
		free(pp->bm);
	}
}

long
horspool_search(Pattern *pp, const unsigned char *str, size_t len)
{
	long offset = 0;
	size_t m = pp->length - 1;

	while (offset + pp->length <= len) {
		long i;
		int err = 0;
		size_t delta = pp->length - pp->max_err;

		INFO("off=%ld str=\"%s\"", offset, str+offset);

		for (i = m; 0 <= i && err <= pp->max_err; i--) {
			INFO(
				"delta=%lu e=%d T='%c' P='%c' m='%c' d=%lu",
				delta, err, str[offset + i], pp->pattern[i],
				str[offset + m], pp->bm[err][str[offset + m]]
			);

			if (str[offset + i] != pp->pattern[i]) {
				delta = min(delta, pp->bm[err][str[offset + m]]);
				delta = min(delta, pp->bm[err][str[offset + m -1]]);
				err++;
			}
		}

		if (err <= pp->max_err) {
			INFO("return offset=%ld", offset);
			return offset;
		}
		offset += delta;
	}

	INFO("return -1 no match");
	return -1;
}

/*
 * Boyer-Moore-Horspool-Sunday search algorithm (quick search variant).
 *
 * https://csclub.uwaterloo.ca/~pbarfuss/p132-sunday.pdf
 * http://www-igm.univ-mlv.fr/~lecroq/string/node19.html#SECTION00190
 * http://alg.csie.ncnu.edu.tw/course/StringMatching/Quick%20Searching.ppt
 */
int
sunday_init(Pattern *pp, const unsigned char *pattern, unsigned max_err)
{
	long i, k;

	pp->max_err = max_err;
	pp->pattern = pattern;
	pp->length = strlen((char *)pattern);

	if (pp->length < max_err) {
		errno = EINVAL;
		return -1;
	}

	if ((pp->qs = malloc((max_err+1) * sizeof (*pp->qs))) == NULL) {
		return -1;
	}

	for (k = 0; k <= max_err; k++) {
		for (i = 0; i < (sizeof (*pp->qs) / sizeof (**pp->qs)); i++) {
			pp->qs[k][i] = pp->length + 1 - k;
		}
		for (i = 0; i < pp->length - k; i++) {
			pp->qs[k][pattern[i]] = pp->length - i - k;
			DEBUG("e=%ld ch='%c' delta=%lu", k, pattern[i], pp->qs[k][pattern[i]]);
		}
		DEBUG("e=%ld ch=* delta=%lu", k, pp->qs[k][0]);
	}

	return 0;
}

void
sunday_fini(Pattern *pp)
{
	if (pp != NULL && pp->qs != NULL) {
		free(pp->qs);
	}
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
		long i;
		int err = 0;
		size_t delta = pp->length + 1 - pp->max_err;

		INFO("off=%ld str=\"%s\"", offset, str+offset);

		/* Sunday algorithm can scan any order. */
		for (i = 0; i < pp->length && err <= pp->max_err; i++) {
			INFO(
				"delta=%lu e=%d T='%c' P='%c' m='%c'",
				delta, err, str[offset + i], pp->pattern[i],
				str[offset + pp->length - err]
			);

			if (str[offset + i] != pp->pattern[i]) {
				delta = min(delta, pp->qs[err][str[offset + pp->length - err]]);
				err++;
			}
		}

		if (err <= pp->max_err) {
			INFO("return offset=%ld", offset);
			return offset;
		}
		offset += delta;
	}

	INFO("return -1 no match");
	return -1;
}

/*
 * http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.14.703&rep=rep1&type=pdf
 *	Experiments with a Very Fast Substring Search Algorithm
 *	P. D. SMITH; SOFTWARE—PRACTICE AND EXPERIENCE, VOL. 21(10), 1065–1074 (OCTOBER 1991)
 *
 * http://www-igm.univ-mlv.fr/~lecroq/string/node21.html
 */
int
smith_init(Pattern *pp, const unsigned char *pattern, unsigned max_err)
{
	return horspool_init(pp, pattern, max_err) < 0
	|| sunday_init(pp, pattern, max_err) < 0
	? -1 : 0;
}

void
smith_fini(Pattern *pp)
{
	horspool_fini(pp);
	sunday_fini(pp);
}

long
smith_search(Pattern *pp, const unsigned char *str, size_t len)
{
	long offset = 0;

	/* Note that this can reference the NUL byte when "offset
	 * + pp->length == len", which is not an index bounds error
	 * in C, but when ported to other languages like Java or C#
	 * that have no sentinel end of string byte, this has to be
	 * handled specially.
	 */
	while (offset + pp->length <= len) {
		long i;
		int err = 0;
		size_t delta = pp->length + 1 - pp->max_err;

		INFO("off=%ld str=\"%s\"", offset, str+offset);

		/* Sunday algorithm can scan any order. */
		for (i = 0; i < pp->length && err <= pp->max_err; i++) {
			INFO(
				"delta=%lu e=%d T='%c' P='%c' m='%c'",
				delta, err, str[offset + i], pp->pattern[i],
				str[offset + pp->length - err]
			);

			if (str[offset + i] != pp->pattern[i]) {
				int shift = max(
					pp->bm[err][str[offset + pp->length - 1 - err]],
					pp->qs[err][str[offset + pp->length - err]]
				);
				delta = min(delta, shift);
				err++;
			}
		}

		if (err <= pp->max_err) {
			INFO("return offset=%ld", offset);
			return offset;
		}
		offset += delta;
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
"usage: search [-bv][-a 0|1|2][-k num] string file ...\n"
"-a 0|1|2\t0 = horspool, 1 = sunday (*), 2 = smith\n"
"-b\t\tbracket the first match\n"
"-k num\t\tmax. k-mismatches (0)\n"
"-v\t\tverbose debug\n"
;

typedef struct {
	int (*fn_init)(Pattern *, const unsigned char *, unsigned);
	long (*fn_srch)(Pattern *, const unsigned char *, size_t);
	void (*fn_fini)(Pattern *);
} Search;

static Search srch_alg[] = {
	{ horspool_init, horspool_search, horspool_fini },
	{ sunday_init, sunday_search, sunday_fini },
	{ smith_init, smith_search, smith_fini },
	{ NULL, NULL, NULL }
};

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
	unsigned max_err;
	long lineno, offset;
	int brackets, ch, rc, argi;
	unsigned char line[LINE_SIZE];
	Search *alg = &srch_alg[1];

	max_err = 0;
	brackets = 0;

	while ((ch = getopt(argc, argv, "a:bk:v")) != -1) {
		switch (ch) {
		case 'a':
			alg = &srch_alg[strtoul(optarg, NULL, 10)];
			break;
		case 'b':
			brackets = 1;
			break;
		case 'k':
			max_err = strtoul(optarg, NULL, 10);
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
	alg->fn_init(&pat, (const unsigned char *)argv[optind], max_err);

	for (argi = optind+1; argi < argc; argi++) {
		if ((fp = fopen(argv[argi], "r")) == NULL) {
			(void) fprintf(stderr, "%s: %s (%d)\n", argv[argi], strerror(errno), errno);
			continue;
		}

		lineno = 0;
		while (0 < (line_len = inputline(fp, line, sizeof (line)))) {
			lineno++;
			if ((offset = alg->fn_srch(&pat, line, line_len)) != -1) {
				if (optind+2 < argc)
					(void) printf("%s: ", argv[argi]);
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

	alg->fn_fini(&pat);

	return rc;
}

#endif
