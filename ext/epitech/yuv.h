#ifndef		__YUV_H
#define		__YUV_H

#include <stdlib.h>


unsigned char *yuv422 (const unsigned char *rgb, const int rows, const int cols);
unsigned char *rgb422 (const unsigned char *yuv, const int rows, const int cols);

#endif
