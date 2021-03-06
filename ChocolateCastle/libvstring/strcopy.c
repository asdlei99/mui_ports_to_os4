/* libvstring/strcopy.c */

#include "libvstring.h"


STRPTR StrCopy(CONST_STRPTR s, STRPTR d)
{
	while ((*d++ = *s++) != '\0');

	return (--d);
}

