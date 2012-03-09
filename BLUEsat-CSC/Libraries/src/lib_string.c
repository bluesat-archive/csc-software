/*
 * A replacement for the standard library string.h
 *
 */

#include "lib_string.h"

/**
* memset - Fill a region of memory with the given value
* @s: Pointer to the start of the area.
* @c: The byte to fill the area with
* @count: The size of the area.
*
* Do not use memset() to access IO space, use memset_io() instead.
*/

void * memset(void * s, int c, unsigned long count)
{
	char *xs = (char *) s;

	while (count--)
		*xs++ = c;

	return s;
}

/**
* strncpy - Copy a length-limited, %NUL-terminated string
* @dest: Where to copy the string to
* @src: Where to copy the string from
* @count: The maximum number of bytes to copy
*
* Note that unlike userspace strncpy, this does not %NUL-pad the buffer.
* However, the result is not %NUL-terminated if the source exceeds
* @count bytes.
*/

char * strncpy(char * dest, const char *src, unsigned long count)
{
	char *tmp = dest;

	while (count-- && (*dest++ = *src++) != '\0')
		/* nothing */;

	return tmp;
}
