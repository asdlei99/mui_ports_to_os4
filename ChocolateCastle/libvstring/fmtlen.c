/* libvstring/fmtlen.c */

#include "libvstring.h"

#include <proto/exec.h>
#if defined(__MORPHOS__)
#include <exec/rawfmt.h>
#endif

ULONG FmtLen(STRPTR fmt, ...)
{
	va_list args;
	ULONG l = 0;

	va_start(args, fmt);
	VNewRawDoFmt(fmt, (APTR(*)(APTR, UBYTE))RAWFMTFUNC_COUNT, (STRPTR)&l, args);
	va_end(args);
	return l;
}

