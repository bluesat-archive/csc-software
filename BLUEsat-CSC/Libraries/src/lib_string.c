/*
 * A replacement for the standard library string.h
 *
 */

#include "lib_string.h"

void * memset(void * s, int c, unsigned long count)
{
	char *xs = (char *) s;

	while (count--)
		*xs++ = c;

	return s;
}

char * strncpy(char * dest, const char *src, unsigned long count)
{
	char *tmp = dest;

	while (count-- && (*dest++ = *src++) != '\0')
		/* nothing */;

	return tmp;
}

void * memcpy(void * dest, const void *src, unsigned long count)
{
	char *tmp = (char *) dest, *s = (char *) src;

	while (count--)
		*tmp++ = *s++;

	return dest;
}

#define NUM_ASCII_OFFSET	'0'
#define ALPHA_ASCII_OFFSET	'A'
#define DECIMAL_BASE		10
char cValToHex(unsigned char ucValue)
{
	if (ucValue < DECIMAL_BASE)
	{
		return NUM_ASCII_OFFSET + ucValue;
	}
	else
	{
		return ALPHA_ASCII_OFFSET + (ucValue - DECIMAL_BASE);
	}
}
