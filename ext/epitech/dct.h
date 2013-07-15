#ifndef		__DCT_H
#define		__DCT_H

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define PI 3.14159265358979
#define PI_DIVIDED_16 0.19634954084
#define INVERSE_SQRT_2 0.70710678118

#define COEFF_U(Cu, u) { \
  if (u == 0) Cu = INVERSE_SQRT_2; else Cu = 1.0; \
}
#define COEFF_V(Cv, v) { \
  if (v == 0) Cv = INVERSE_SQRT_2; else Cv = 1.0; \
}

static int quantum_matrix[8][8] = {
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

char *dct_encode (const unsigned char *input, const int height, const int width);
unsigned char *dct_decode (const char *input, const int height, const int width);

#endif
