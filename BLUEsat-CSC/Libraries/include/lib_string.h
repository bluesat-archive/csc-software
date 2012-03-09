/*
 * A replacement for the standard library string.h
 *
 */

#ifndef LIB_STRING_H_
#define LIB_STRING_H_

void * memset(void * s, int c, unsigned long count);

char * strncpy(char * dest, const char *src, unsigned long count);

#endif /* LIB_STRING_H_ */
