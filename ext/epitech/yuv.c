#include <math.h>

#include "yuv.h"

static unsigned char
clamp (int val)
{
  return (val > 255) ? 255 : (val < 0) ? 0 : val;
}

unsigned char *
yuv422 (const unsigned char *rgb, const int rows, const int cols)
{
  //Because cols * 3 (rgb) and 2/3
  int size = rows * cols * 2;
  int i = 0;
  int j = 0;

  unsigned char *yuv = malloc (sizeof (unsigned char) * size);

  while (j < size) {
    int r1 = rgb[i + 0];
    int g1 = rgb[i + 1];
    int b1 = rgb[i + 2];
    int r2 = rgb[i + 3];
    int g2 = rgb[i + 4];
    int b2 = rgb[i + 5];

    yuv[j + 0] = (0.257 * r1) + (0.504 * g1) + (0.098 * b1) + 16;
    yuv[j + 1] = (0.439 * r1) - (0.368 * g1) - (0.071 * b1) + 128;
    yuv[j + 2] = (0.257 * r2) + (0.504 * g2) + (0.098 * b2) + 16;
    yuv[j + 3] = -(0.148 * r1) - (0.291 * g1) + (0.439 * b1) + 128;

    i += 6;
    j += 4;
  }
  return yuv;
}

unsigned char *
rgb422 (const unsigned char *yuv, const int rows, const int cols)
{
  int size = rows * cols * 3;
  int i = 0;
  int j = 0;

  unsigned char *rgb = malloc (sizeof (unsigned char) * size);

  while (i < size) {
    int y1 = yuv[j + 0];
    int u = yuv[j + 1];
    int y2 = yuv[j + 2];
    int v = yuv[j + 3];

    rgb[i + 0] = clamp (1.164 * (y1 - 16) + 2.018 * (u - 128));
    rgb[i + 1] =
        clamp (1.164 * (y1 - 16) - 0.813 * (v - 128) - 0.391 * (u - 128));
    rgb[i + 2] = clamp (1.164 * (y1 - 16) + 1.596 * (v - 128));
    rgb[i + 3] = clamp (1.164 * (y2 - 16) + 2.018 * (u - 128));
    rgb[i + 4] =
        clamp (1.164 * (y2 - 16) - 0.813 * (v - 128) - 0.391 * (u - 128));
    rgb[i + 5] = clamp (1.164 * (y2 - 16) + 1.596 * (v - 128));

    i += 6;
    j += 4;
  }
  return rgb;
}