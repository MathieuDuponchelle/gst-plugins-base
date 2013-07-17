#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "dct.h"

const double idct_cos[8][8] = {
  {1.000000, 0.980785, 0.923880, 0.831470, 0.707107, 0.555570, 0.382683,
      0.195090},
  {1.000000, 0.831470, 0.382683, -0.195090, -0.707107, -0.980785, -0.923880,
      -0.555570},
  {1.000000, 0.555570, -0.382683, -0.980785, -0.707107, 0.195090, 0.923880,
      0.831470},
  {1.000000, 0.195090, -0.923880, -0.555570, 0.707107, 0.831470, -0.382683,
      -0.980785},
  {1.000000, -0.195090, -0.923880, 0.555570, 0.707107, -0.831470, -0.382683,
      0.980785},
  {1.000000, -0.555570, -0.382683, 0.980785, -0.707107, -0.195090, 0.923880,
      -0.831470},
  {1.000000, -0.831470, 0.382683, 0.195090, -0.707107, 0.980785, -0.923880,
      0.555570},
  {1.000000, -0.980785, 0.923880, -0.831470, 0.707107, -0.555570, 0.382683,
      -0.195090}
};

static void
idct (unsigned char *dst, double data[8][8], const int xpos,
    const int ypos, const int width)
{
  double Cu, Cv;
  unsigned int pre_pos_y;

  /* iDCT */
  for (int y = 0; y < 8; y++) {
    pre_pos_y = (y + ypos) * width;

    for (int x = 0; x < 8; x++) {
      double z = 0.0;

      for (int v = 0; v < 8; v++) {
        COEFF_V (Cv, v);
        for (int u = 0; u < 8; u++) {

          COEFF_U (Cu, u);
          z += Cu * Cv * data[v][u] * idct_cos[x][u] * idct_cos[y][v];
        }
      }

      z /= 4.0;
      if (z > 255.0)
        z = 255.0;
      if (z < 0)
        z = 0.0;

      dst[pre_pos_y + (x + xpos)] = (unsigned char) z;
    }
  }
}


/*
** Read a 8*8 block and dequantize at the same time
*/
static void
read_from_buff (const char *src, double dst[8][8], const int block_num)
{
  int buff_index = 0;

  for (int x = 0; x < 8; x++)
    for (int y = 0; y < 8; y++) {
      dst[x][y] = src[buff_index + block_num] * quantum_matrix[x][y];
      buff_index++;
    }
}

unsigned char *
dct_decode (const char *input, const int height, const int width)
{
  unsigned char *res = malloc (height * width);
  double block[8][8];
  int block_num = 0;
  int tmp_i;

  for (int i = 0; i < height / 8; i++) {
    tmp_i = i * 8;
    for (int j = 0; j < width / 8; j++) {
      read_from_buff (input, block, block_num * 64);
      idct (res, block, j * 8, tmp_i, width);
      block_num++;
    }
  }
  return res;
}
