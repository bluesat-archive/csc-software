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

/**
* memcpy - Copy one area of memory to another
* @dest: Where to copy to
* @src: Where to copy from
* @count: The size of the area.
*
* You should not use this function to access IO space, use memcpy_toio()
* or memcpy_fromio() instead.
*/

void * memcpy(void * dest, const void *src, unsigned long count)
{
	char *tmp = (char *) dest, *s = (char *) src;

	while (count--)
		*tmp++ = *s++;

	return dest;
}

