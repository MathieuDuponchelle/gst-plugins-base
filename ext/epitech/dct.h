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

static int quantum_matrix[8][8] = {
  {3, 6, 9, 12, 15, 18, 21, 24},
  {6, 12, 18, 24, 30, 36, 42, 48},
  {9, 18, 27, 36, 45, 54, 63, 72},
  {12, 24, 36, 48, 60, 72, 84, 96},
  {15, 30, 45, 60, 75, 90, 105, 120},
  {18, 36, 54, 72, 90, 108, 126, 144},
  {21, 42, 63, 84, 105, 126, 147, 168},
  {24, 48, 72, 96, 120, 144, 168, 192}
};

static int quantum_matrix_two[8][8] = {
  {16, 11, 10, 16, 25, 40, 51, 61},
  {12, 12, 14, 19, 26, 58, 60, 55},
  {14, 13, 16, 24, 40, 57, 69, 56},
  {14, 17, 22, 29, 51, 87, 80, 62},
  {18, 22, 37, 56, 68, 109, 103, 77},
  {24, 35, 55, 64, 81, 104, 113, 92},
  {49, 64, 79, 87, 103, 121, 120, 101},
  {72, 92, 95, 98, 112, 100, 103, 99}
};


/* system dependent types */
typedef unsigned char byte_t;   /* unsigned 8 bit */
typedef unsigned char code_t;   /* unsigned 8 bit for character codes */
typedef unsigned int count_t;   /* unsigned 32 bit for character counts */

char *dct_encode (unsigned char *input, const int height, const int width);
unsigned char *dct_decode (char *input, const int height, const int width);

#endif
