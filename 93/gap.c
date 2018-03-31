/*
 * gap.c		
 *
 * Anthony's Editor July 93
 *
 * Copyright 1993, 1993 by Anthony Howe.  All rights reserved.  No warranty.
 */

#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "header.h"

typedef struct t_undo {
	short u_set;
	t_point u_point;
	t_point u_gap;
	t_point u_egap;
} t_undo;

static t_undo ubuf;

/*
 *	Enlarge the gap by n characters.  
 *	Note that the position of the gap cannot change.
 */
int
growgap(n)
t_point n;
{
	t_char *new;
	t_point buflen, newlen, xgap, xegap;
	
	assert(buf <= gap);
	assert(gap <= egap);
	assert(egap <= ebuf);

	xgap = gap - buf;
	xegap = egap - buf;
	buflen = ebuf - buf;
	newlen = buflen + n * sizeof (t_char);

	if (buflen == 0) {
		if (newlen < 0 || MAX_SSIZE_T < newlen)
			fatal(f_alloc);
		new = (t_char*) malloc((size_t) newlen);
		if (new == NULL)
			/* Cannot edit a file without a buffer. */
			fatal(f_alloc);
	} else {
		if (newlen < 0 || MAX_SSIZE_T < newlen) {
			msg(m_alloc);
			return (FALSE);
		}
		new = (t_char*) realloc(buf, (size_t) newlen);
		if (new == NULL) {
			/* Report non-fatal error. */
			msg(m_alloc);
			return (FALSE);
		}
	}

	/* Relocate pointers in new buffer and append the new 
	 * extension to the end of the gap. 
	 */
	buf = new;
	gap = buf + xgap;	
	ebuf = buf + buflen; 
	egap = buf + newlen;
	while (xegap < buflen--)
		*--egap = *--ebuf;
	ebuf = buf + newlen;

	assert(buf < ebuf);		/* Buffer must exist. */
	assert(buf <= gap);
	assert(gap < egap);		/* Gap must grow only. */
	assert(egap <= ebuf);

	return (TRUE);
}

/*
 *
 */
t_point
movegap(offset)
t_point offset;
{
	t_char *p = ptr(offset);
	while (p < gap)
		*--egap = *--gap;
	while (egap < p)
		*gap++ = *egap++;
	assert(gap <= egap);
	assert(buf <= gap);
	assert(egap <= ebuf);
	return (pos(egap));
}

/*
 * Return pointer on range buf <= ptr(offset) <= ebuf.
 */
t_char *
ptr(offset)
register t_point offset;
{
	register t_char *cp;

	if (offset < 0)
		return (buf);
	cp = buf + offset;
	if (gap <= cp)
		cp += (unsigned long)(egap - gap);
	if (ebuf <= cp)
		return (ebuf);
	return (cp);
}

/*
 * Return offset on range 0 <= pos(pointer) <= pos(ebuf).
 */
t_point
pos(cp)
register t_char *cp;
{
	register t_point offset;

	offset = (t_point)(cp - buf);
	if (cp < buf)
		return (0);
	if (ebuf <= cp)
		cp = ebuf;
	if (egap <= cp)
		offset -= (unsigned long)(egap - gap);
	return (offset);
}

/*
 * Return the current region.
 */
void
getregion(rp)
t_region *rp;
{
	if (marker == NOMARK) {
		rp->left = rp->right = point;
	} else if (point <= marker) {
		rp->left = point;
		rp->right = marker-1;
	} else {
		rp->left = marker;
		rp->right = point-1;
	}
}

int
posix_file(fn)
char *fn;
{
	if (fn[0] == '\0' || fn[0] == '_')
		return (FALSE);
#ifdef DRIVE_COLON
	if (fn[1] == ':') {
 		if (!isalpha(fn[0]))
			return (FALSE);
		fn += 2;
	}
#endif /* DRIVE_COLON */
	for (; *fn != '\0'; ++fn) {
		if (!isalnum(*fn) && *fn != '.' && *fn != '_' && *fn != '-' 
#ifdef EITHER_SLASH
		&& *fn != '/' && *fn != '\\')
#else
		&& *fn != '/')
#endif /* EITHER_SLASH */
			return (FALSE);
	}
	return (TRUE);
}

/*
 *
 */
int
save(fn)
char *fn;
{
	int fd;
	t_point llen, rlen;
	t_char *lhalf, *rhalf;

	if (!posix_file(fn)) {
		msg(m_badname);
		return (FALSE);
	}

	if ((fd = open(fn, O_WRONLY|O_CREAT|O_TRUNC, FILE_MODE)) < 0) {
		msg(m_open, fn);
		return (FALSE);
	}
	if (marker == NOMARK || point == marker) {
		lhalf = buf;
		llen = (t_point) (gap - buf);
		rhalf = egap;
		rlen = (t_point) (ebuf - egap);
	} else {
		/* When saving only a block of text, we'll move the gap
		 * to the start of the block, and set the left-half to
		 * zero length.  We should never see the error message
		 * in the saved block of text. 
		 */
		llen = 0;
		lhalf = (t_char *) m_error;
		if (point < marker) {
			(void) movegap(point);
			rlen = marker - point;
		} else {
			(void) movegap(marker);
			rlen = point - marker;
		}
		rhalf = egap;
	}

	if (write(fd, lhalf, (size_t) llen) != llen
	|| write(fd, rhalf, (size_t) rlen) != rlen) {
		msg(m_write, fn);
		return (FALSE);
	}
	if (close(fd) != 0) {
		msg(m_close, fn);
		return (FALSE);
	}

	if (lhalf != (t_char *) m_error)
		modified = FALSE;
	msg(m_saved, fn, llen + rlen);
	return (TRUE);
}

/*
 *
 */
int
load(fn)
char *fn;
{
	int fd;
	SSIZE_T len;
	struct stat sb;

	if (stat(fn, &sb) < 0) {
		msg(m_stat, fn);
		return (FALSE);
	}
	if (MAX_SSIZE_T < sb.st_size) {
		msg(m_toobig, fn);
		return (FALSE);
	}
	if (egap-gap < sb.st_size * sizeof (t_char) && !growgap(sb.st_size))
		return (FALSE);
	if ((fd = open(fn, O_RDONLY)) < 0) {
		msg(m_open, fn);
		return (FALSE);
	}
	point = movegap(point);
	undoset();

	len = read(fd, gap, (size_t) sb.st_size);
	if (close(fd) != 0) {
		msg(m_close, fn);
		return (FALSE);
	}
	if (len < 0) {
		msg(m_read, fn);
		return (FALSE);
	}

	gap += len;
	modified = TRUE;
	msg(m_loaded, fn, len);
	return (TRUE);
}

/*
 *	Record a new undo location.
 */
void
undoset()
{
	ubuf.u_set = TRUE;
	ubuf.u_point = point;
	ubuf.u_gap = gap - buf;
	ubuf.u_egap = egap - buf;
}

/*
 *	Undo.
 */
void
undo()
{
	t_undo tmp;

	if (ubuf.u_set) {
		memcpy(&tmp, &ubuf, sizeof (t_undo));
		undoset();
		point = tmp.u_point;
		gap = buf + tmp.u_gap;
		egap = buf + tmp.u_egap;
	} else {
		msg(m_undo);
	}
}

