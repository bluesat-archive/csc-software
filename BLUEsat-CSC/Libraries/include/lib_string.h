/*
 * A replacement for the standard library string.h
 *
 */

#ifndef LIB_STRING_H_
#define LIB_STRING_H_

void * memset(void * s,int c,size_t count);

char * strncpy(char * dest,const char *src,size_t count);

#endif /* LIB_STRING_H_ */
