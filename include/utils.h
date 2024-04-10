#ifndef __UTILS_H 

#define __UTILS_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>  
#include <float.h> 
#include <sys/time.h>
#include <stdint.h>
#include <sys/stat.h>

extern void     print_error(const char*);
extern int      coords_to_index(const unsigned int, const int, const int);
extern double   get_time();
extern double   time_elapsed(const double);
extern void     reverse(int*,unsigned int,unsigned int);
extern char     strnin(const char*, char**, const size_t);
extern int*     cth_convert(int*, int*, const unsigned int);

#endif