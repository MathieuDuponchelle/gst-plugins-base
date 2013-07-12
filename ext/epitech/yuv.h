#ifndef		__YUV_H
#define		__YUV_H

#include <stdlib.h>


unsigned char *yuv422 (unsigned char *rgb, int rows, int cols);
unsigned char *rgb422 (unsigned char *yuv, int rows, int cols);

#endif
