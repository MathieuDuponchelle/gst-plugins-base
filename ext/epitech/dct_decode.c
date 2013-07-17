#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "dct.h"

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

static void
idct (unsigned char *dst, double data[8][8], const int xpos,
    const int ypos, const int width)
{
  double Cu, Cv;
  double pre_cos_y;
  double pre_cos_x;
  double cos_v;
  unsigned int pre_pos_y;

  /* iDCT */
  for (int y = 0; y < 8; y++) {
    pre_cos_y = (2 * y + 1) * PI_DIVIDED_16;
    pre_pos_y = (y + ypos) * width;

    for (int x = 0; x < 8; x++) {
      double z = 0.0;
      pre_cos_x = (2 * x + 1) * PI_DIVIDED_16;

      for (int v = 0; v < 8; v++) {
        COEFF_V (Cv, v);
        cos_v = cos (pre_cos_y * v);

        for (int u = 0; u < 8; u++) {

          COEFF_U (Cu, u);
          z += Cu * Cv * data[v][u] * cos ((double) u * pre_cos_x) * cos_v;
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
      idct (res, (const double (*)[8]) block, j * 8, tmp_i, width);
      block_num++;
    }
  }
  return res;
}
