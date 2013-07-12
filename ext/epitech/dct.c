#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "dct.h"

/* DCT rows and columns separately
 *  *
 *   * C(i) = a(i)/2 * sum for x=0 to N-1 of
 *    *   s(x) * cos( pi * i * (2x + 1) / 2N )
 *     *
 *      * if x = 0, a(x) = 1/sqrt(2)
 *       *      else a(x) = 1
 *        */

void
dct_1d (double *in, double *out, const int count)
{
  int x, u;

  for (u = 0; u < count; u++) {
    double z = 0;

    for (x = 0; x < count; x++) {
      z += in[x] * cos (PI * (double) u * (double) (2 * x + 1)
          / (double) (2 * count));
    }

    if (u == 0)
      z *= 1.0 / sqrt (2.0);
    out[u] = z / 2.0;
  }
}

void
dct (const unsigned char *src, double data[8][8],
    const int xpos, const int ypos, const int width)
{
  int i, j;
  double in[8], out[8], rows[8][8];

  /* transform rows */
  for (j = 0; j < 8; j++) {
    for (i = 0; i < 8; i++)
      //in[i] = (double) pixel(tga, xpos+i, ypos+j);
      in[i] = (double) src[(ypos + j) * width + (xpos + i)];
    dct_1d (in, out, 8);
    for (i = 0; i < 8; i++)
      rows[j][i] = out[i];
  }

  /* transform columns */
  for (j = 0; j < 8; j++) {
    for (i = 0; i < 8; i++)
      in[i] = rows[i][j];
    dct_1d (in, out, 8);
    for (i = 0; i < 8; i++)
      data[i][j] = out[i];
  }
}

void
quantize (double dct_buf[8][8], char res[8][8])
{
  int x, y;

  for (y = 0; y < 8; y++)
    for (x = 0; x < 8; x++) {
      res[y][x] = dct_buf[y][x] / quantum_matrix_two[y][x];
      if (res[y][x] > 127)
        res[y][x] = 127;
      else if (res[y][x] < -126)
        res[y][x] = -126;
    }
}

void
dquantize (double dct_buf[8][8], char res[8][8])
{
  int x, y;

  for (y = 0; y < 8; y++)
    for (x = 0; x < 8; x++)
      dct_buf[y][x] = res[y][x] * quantum_matrix_two[y][x];
}

void
idct (unsigned char *dst, double data[8][8], const int xpos, const int ypos,
    const int width)
{
  int u, v, x, y;

  /* iDCT */
  for (y = 0; y < 8; y++)
    for (x = 0; x < 8; x++) {
      double z = 0.0;

      for (v = 0; v < 8; v++)
        for (u = 0; u < 8; u++) {
          double S, q;
          double Cu, Cv;

          COEFFS (Cu, Cv, u, v);
          S = data[v][u];

          q = Cu * Cv * S *
              cos ((double) (2 * x + 1) * (double) u * PI / 16.0) *
              cos ((double) (2 * y + 1) * (double) v * PI / 16.0);

          z += q;
        }

      z /= 4.0;
      if (z > 255.0)
        z = 255.0;
      if (z < 0)
        z = 0.0;

      //pixel(tga, x+xpos, y+ypos) = (uint8_t) z;
      dst[(y + ypos) * width + (x + xpos)] = (unsigned char) z;
    }
}


/*
int main()
{
	tga_image tga;
	double dct_buf[8][8];
	int i, j, k, l;

	load_tga(&tga, "in.tga");

	k = 0;
	l = (tga.height / 8) * (tga.width / 8);
	for (j=0; j<tga.height/8; j++)
		for (i=0; i<tga.width/8; i++)
		{
			dct(&tga, dct_buf, i*8, j*8);
			quantize(dct_buf);
			idct(&tga, dct_buf, i*8, j*8);
			printf("processed %d/%d blocks.\r", ++k,l);
			fflush(stdout);
		}
	printf("\n");

	DONTFAIL( tga_write_mono("out.tga", tga.image_data,
				tga.width, tga.height) );

	tga_free_buffers(&tga);
	return EXIT_SUCCESS;
}
*/
void
print_table (double table[8][8])
{
  for (int i = 0; i < 8; ++i)
    for (int j = 0; j < 8; ++j)
      printf ("%lf, ", table[i][j]);
  printf ("\n");
}

void
write_to_buff (char *dst, char src[8][8], int block_num)
{
  int buff_index = 0;

  for (int x = 0; x < 8; x++)
    for (int y = 0; y < 8; y++) {
      dst[buff_index + block_num] = src[x][y];
      buff_index++;
    }
}

void
read_from_buff (char *src, char dst[8][8], int block_num)
{
  int buff_index = 0;

  for (int x = 0; x < 8; x++)
    for (int y = 0; y < 8; y++) {
      dst[x][y] = src[buff_index + block_num];
      buff_index++;
    }
}

char *
dct_encode (unsigned char *input, const int height, const int width)
{
  char *res = malloc (sizeof (char) * height * width);
  double tmp_res[8][8] = { 0 };
  char block[8][8] = { 0 };
  int block_num = 0;

  for (int i = 0; i < height / 8; i++)
    for (int j = 0; j < width / 8; j++) {
      dct (input, tmp_res, j * 8, i * 8, width);
      quantize (tmp_res, block);
      write_to_buff (res, block, block_num * 64);
      block_num++;
    }
  return res;
}

unsigned char *
dct_decode (char *input, const int height, const int width)
{
  unsigned char *res = malloc (height * width);
  double tmp_res[8][8] = { 0 };
  char block[8][8];
  int block_num = 0;

  for (int i = 0; i < height / 8; i++)
    for (int j = 0; j < width / 8; j++) {
      read_from_buff (input, block, block_num * 64);
      block_num++;
      dquantize (tmp_res, block);
      idct (res, tmp_res, j * 8, i * 8, width);
    }
  return res;
}

static void
dumpToFile (unsigned char *buffer, const char *fName, count_t count)
{
  int i = 0;
  FILE *fpOut;

  if ((fpOut = fopen (fName, "wb")) == NULL) {
    perror (fName);
    exit (EXIT_FAILURE);
  }
  while (i < count) {
    fputc (buffer[i], fpOut);
    i += 1;
  }
  fclose (fpOut);
}

static unsigned char *
readFile (const char *name, count_t * count)
{
  FILE *file;
  unsigned char *buffer;
  unsigned long fileLen;
  int junk;
  file = fopen (name, "rb");

  if (!file) {
    fprintf (stderr, "Unable to open file %s", name);
    return NULL;
  }
  fseek (file, 0, SEEK_END);
  fileLen = ftell (file);
  fseek (file, 0, SEEK_SET);
  buffer = (unsigned char *) malloc (fileLen + 1);
  if (!buffer) {
    fprintf (stderr, "Memory error!");
    fclose (file);
    return NULL;
  }
  junk = fread (buffer, fileLen, 1, file);
  junk = junk;
  fclose (file);
  *count = fileLen;

  return buffer;
}

#include "yuv.h"

void
main ()
{
  count_t count = 0;
  unsigned char *buffer = readFile ("src.rgb", &count);
  unsigned char *buffer_yuv = yuv422 (buffer, 240, 320);
  char *dct;
  unsigned char *restored;

  dct = dct_encode (buffer_yuv, 240, 320 * 2);
  restored = dct_decode (dct, 240, 320 * 2);
  unsigned char *pute = rgb422 (restored, 240, 320);
  dumpToFile (pute, "srcd.rgb", 240 * 320 * 3);
}

/*
   int
   main (int argc, char *argv[])
   {
   double *encoded_buffer;
   count_t encoded_size;
   unsigned char *decoded_buffer;
   count_t decoded_size;

   encoded_buffer = dct_encode (buffer, 512, 512);
   decoded_buffer = dct_decode (encoded_buffer, 512, 512);

   return (0);
   }*/
