#include <math.h>
#include <stdio.h>

#include "utils.h"
#include "yuv.h"

/*
http://www.fourcc.org/fccyvrgb.php

Note for yuvtorgb :
In both these cases, you have to clamp the output values to keep them in the [0-255] range.
Rumour has it that the valid range is actually a subset of [0-255] (I've seen an RGB range of [16-235] mentioned)
but clamping the values into [0-255] seems to produce acceptable results to me.
*/

static void
rgbtoyuv (unsigned char r, unsigned char g, unsigned char b, unsigned char *y,
    unsigned char *u, unsigned char *v)
{
  *y = (0.257 * r) + (0.504 * g) + (0.098 * b) + 16;
  *u = (0.439 * r) - (0.368 * g) - (0.071 * b) + 128;
  *v = -(0.148 * r) - (0.291 * g) + (0.439 * b) + 128;
}

static void
yuvtorgb (unsigned char *r, unsigned char *g, unsigned char *b, unsigned char y,
    unsigned char u, unsigned char v)
{
  int rt = 1.164 * (y - 16) + 2.018 * (u - 128);
  int gt = 1.164 * (y - 16) - 0.813 * (v - 128) - 0.391 * (u - 128);
  int bt = 1.164 * (y - 16) + 1.596 * (v - 128);

  // Clamping (trimming) values as the note tell's us to do.
  *r = (rt > 255) ? 255 : (rt < 0) ? 0 : rt;
  *g = (gt > 255) ? 255 : (gt < 0) ? 0 : gt;
  *b = (bt > 255) ? 255 : (bt < 0) ? 0 : bt;
}

static unsigned char
clamp (int val)
{
  return (val > 255) ? 255 : (val < 0) ? 0 : val;
}

unsigned char *
yuv422 (unsigned char *rgb, int rows, int cols)
{
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
rgb422 (unsigned char *yuv, int rows, int cols)
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

// Keep in mind that OpenCV use BGR not RGB
/*
unsigned char* toYuv(unsigned char* rgb, int rows, int cols)
{
    int size = rows * cols * 3;
    unsigned char* yuv = xmalloc(sizeof(unsigned char) * size);

    for (int i = 0; i < size; i += 3)
    {
        rgbtoyuv(
            rgb[i + 2],
            rgb[i + 1],
            rgb[i + 0],
            &(yuv[i + 0]),
            &(yuv[i + 1]),
            &(yuv[i + 2])
        );
    }
    return yuv;
}

unsigned char* toRgb(unsigned char* yuv, int rows, int cols)
{
    int size = rows * cols * 3;
    unsigned char* rgb = xmalloc(sizeof(unsigned char) * size);

    for (int i = 0; i < size; i += 3)
    {
        yuvtorgb(
            &(rgb[i + 2]),
            &(rgb[i + 1]),
            &(rgb[i + 0]),
            yuv[i + 0],
            yuv[i + 1],
            yuv[i + 2]
        );
    }
    return rgb;
}
*/
