#ifndef		__DCT_H
#define		__DCT_H

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define PI 3.14159265358979

#define COEFFS(Cu,Cv,u,v) { \
	if (u == 0) Cu = 1.0 / sqrt(2.0); else Cu = 1.0; \
	if (v == 0) Cv = 1.0 / sqrt(2.0); else Cv = 1.0; \
}

/* system dependent types */
typedef unsigned char byte_t;   /* unsigned 8 bit */
typedef unsigned char code_t;   /* unsigned 8 bit for character codes */
typedef unsigned int count_t;   /* unsigned 32 bit for character counts */

double *dct_encode (unsigned char *input, const int rows, const int cols);
unsigned char *dct_decode (double *input, const int rows, const int cols);

#endif
