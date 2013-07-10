#include "dct.h"

static void dct_1d (double *in, double *out, const int count);

void
dct (const unsigned char *input, double data[8][8], const int xpos,
    const int ypos, int cols)
{
  int i, j;
  double in[8], out[8], rows[8][8];

  /* transform rows */
  for (j = 0; j < 8; j++) {
    for (i = 0; i < 8; i++)
      in[i] = (double) input[(ypos + j * cols) + (xpos + i)];
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
quantize (double dct_buf[8][8])
{
  int x, y;

  for (y = 0; y < 8; y++)
    for (x = 0; x < 8; x++)
      if (x > 3 || y > 3)
        dct_buf[y][x] = 0.0;
}

void
idct (unsigned char *output, double data[8][8], const int xpos, const int ypos,
    const int cols)
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
      output[(ypos + y * cols) + (xpos + x)] = (unsigned char) z;
    }
}

void
write_to_char_buffer (unsigned char *dst, double src[8][8])
{
  for (int i = 0; i < 8; ++i)
    for (int j = 0; j < 8; ++j)
      dst[i + j] = src[i][j];
}

void
write_to_double_buffer (double *dst, double src[8][8])
{
  for (int i = 0; i < 8; ++i)
    for (int j = 0; j < 8; ++j)
      dst[i + j] = src[i][j];
}

void
read_from_buffer (double *src, double dst[8][8])
{
  for (int i = 0; i < 8; ++i)
    for (int j = 0; j < 8; ++j)
      dst[i][j] = src[i + j];
}

double *
dct_encode (unsigned char *input, const int rows, const int cols)
{
  double partial_res[8][8] = { 0 };
  double *res = malloc (rows * cols * sizeof (double));
  double *cpy = res;

  for (int i = 0; i < rows / 8; ++i)
    for (int j = 0; j < cols / 8; ++j) {
      dct (input, partial_res, i * 8, j * 8, cols);
      quantize (partial_res);
      write_to_double_buffer (res, partial_res);
      res += 64;
    }

  return cpy;
}

unsigned char *
dct_decode (double *input, const int rows, const int cols)
{
  double partial_res[8][8] = { 0 };
  unsigned char *res = malloc (rows * cols * sizeof (unsigned char));
  unsigned char *cpy = res;

  for (int i = 0; i < rows / 8; ++i)
    for (int j = 0; j < cols / 8; ++j) {
      read_from_buffer (input, partial_res);
      idct (res, partial_res, i * 8, j * 8, cols);
      res += 64;
      input += 64;
    }

  return cpy;
}

static void
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

#define ENABLE_MAIN
#ifdef ENABLE_MAIN


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

int
main (int argc, char *argv[])
{
  count_t count = 0;
  unsigned char *buffer = readFile ("src.rgb", &count);
  double *encoded_buffer;
  count_t encoded_size;
  unsigned char *decoded_buffer;
  count_t decoded_size;

  encoded_buffer = dct_encode (buffer, 512, 512);
  decoded_buffer = dct_decode (encoded_buffer, 512, 512);
  dumpToFile (decoded_buffer, "srcd.rgb", 512 * 512);

  return (0);
}
#endif
