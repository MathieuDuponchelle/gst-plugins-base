#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "dct.h"

const double dct_ld_cos[8][8] = {
  {1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
      1.000000},
  {0.980785, 0.831470, 0.555570, 0.195090, -0.195090, -0.555570, -0.831470,
      -0.980785},
  {0.923880, 0.382683, -0.382683, -0.923880, -0.923880, -0.382683, 0.382683,
      0.923880},
  {0.831470, -0.195090, -0.980785, -0.555570, 0.555570, 0.980785, 0.195090,
      -0.831470},
  {0.707107, -0.707107, -0.707107, 0.707107, 0.707107, -0.707107, -0.707107,
      0.707107},
  {0.555570, -0.980785, 0.195090, 0.831470, -0.831470, -0.195090, 0.980785,
      -0.555570},
  {0.382683, -0.923880, 0.923880, -0.382683, -0.382683, 0.923880, -0.923880,
      0.382683},
  {0.195090, -0.555570, 0.831470, -0.980785, 0.980785, -0.831470, 0.555570,
      -0.195090}
};

/* DCT rows and columns separately
 *  *
 *   * C(i) = a(i)/2 * sum for x=0 to N-1 of
 *    *   s(x) * cos( pi * i * (2x + 1) / 2N )
 *     *
 *      * if x = 0, a(x) = 1/sqrt(2)
 *       *      else a(x) = 1
 *        */

static void
dct_1d (const double *in, double *out, const int count)
{
  for (int u = 0; u < count; u++) {
    double z = 0;

    for (int x = 0; x < count; x++) {
      z += in[x] * dct_ld_cos[u][x];
    }

    if (u == 0)
      z *= INVERSE_SQRT_2;
    out[u] = z / 2.0;
  }
}

static void
dct (const unsigned char *src, double data[8][8],
    const int xpos, const int ypos, const int width)
{
  double in[8], out[8], rows[8][8];

  /* transform rows */
  for (int j = 0; j < 8; j++) {
    int pos_j = (ypos + j) * width;

    for (int i = 0; i < 8; i++)
      in[i] = src[pos_j + (xpos + i)];
    dct_1d (in, out, 8);
    for (int i = 0; i < 8; i++)
      rows[j][i] = out[i];
  }

  /* transform columns */
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++)
      in[i] = rows[i][j];
    dct_1d (in, out, 8);
    for (int i = 0; i < 8; i++)
      data[i][j] = out[i];
  }
}

static inline char
quantize (const double val)
{
  if (val > 127)
    return 127;
  else if (val < -126)
    return -126;
  return (char) val;
}

/*
** Read and quantize
*/
static void
write_to_buff (char *dst, double src[8][8], int block_num)
{
  int buff_index = 0;

  for (int x = 0; x < 8; x++)
    for (int y = 0; y < 8; y++) {
      dst[buff_index + block_num] = quantize (src[x][y] / quantum_matrix[x][y]);
      buff_index++;
    }
}

char *
dct_encode (const unsigned char *input, const int height, const int width)
{
  char *res = malloc (height * width);
  double block[8][8] = { 0 };
  int block_num = 0;

  for (int i = 0; i < height / 8; i++)
    for (int j = 0; j < width / 8; j++) {
      dct (input, block, j * 8, i * 8, width);
      write_to_buff (res, block, block_num * 64);
      block_num++;
    }
  return res;
}
