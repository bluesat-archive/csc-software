/*
 * A replacement for the standard library string.h
 *
 */

#ifndef LIB_STRING_H_
#define LIB_STRING_H_

/**
* memset - Fill a region of memory with the given value
* @s: Pointer to the start of the area.
* @c: The byte to fill the area with
* @count: The size of the area.
*
* Do not use memset() to access IO space, use memset_io() instead.
*/
void * memset(void * s, int c, unsigned long count);

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
char * strncpy(char * dest, const char *src, unsigned long count);

/**
* memcpy - Copy one area of memory to another
* @dest: Where to copy to
* @src: Where to copy from
* @count: The size of the area.
*
* You should not use this function to access IO space, use memcpy_toio()
* or memcpy_fromio() instead.
*/
void * memcpy(void * dest, const void *src, unsigned long count);

/**
* memcmp - Compare memory to see if the data is identical
* @s1: Location 1
* @s2: Location 1
* @size: The size of the area.
*
*/
int memcmp(const void * s1, const void * s2, unsigned long size);

/**
 * Convert value to hex decimal
 */
char cValToHex(unsigned char ucValue);

/**
 * Convert Decimal value string to value
 */
unsigned long ulDeciStringToVal(char *			pcString,
								unsigned char 	ucLength);

#endif /* LIB_STRING_H_ */
