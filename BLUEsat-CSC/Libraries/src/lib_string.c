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


int memcmp(const void * s1, const void * s2, unsigned long size)
{
   unsigned char u1, u2;

   for ( ; size-- ; s1++, s2++) {
      u1 = * (unsigned char *) s1;
      u2 = * (unsigned char *) s2;
      if ( u1 != u2) {
          return (u1-u2);
      }
   }
   return 0;
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

unsigned long ulDeciStringToVal(char *			pcString,
								unsigned char 	ucLength)
{
	unsigned long ulFactor;
	unsigned long ulConvertedValue;

	for (ulFactor = 1, pcString--, ulConvertedValue = 0;
		ucLength > 0;
		ucLength--, ulFactor *= DECIMAL_BASE)
	{
		ulConvertedValue += (pcString[ucLength] - '0') * ulFactor;
	}

	return ulConvertedValue;
}

