#include <stdio.h>
#include <stdlib.h>


#include "dct.h"
#include "yuv.h"
#include "rle.h"

static void
print_table (double table[8][8])
{
  for (int i = 0; i < 8; ++i)
    for (int j = 0; j < 8; ++j)
      printf ("%lf, ", table[i][j]);
  printf ("\n");
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
main (int ac, char **av)
{
  count_t count = 0;
  unsigned char *buffer = readFile ("src.rgb", &count);
  unsigned char *buffer_yuv = yuv422 (buffer, 240, 320);
  char *dct;
  char *dct_two = malloc (240 * 320 * 3);
  unsigned char *restored;
  unsigned char *rle = malloc (240 * 320 * 2);
  unsigned int size = 240 * 320 * 2;

  dct = dct_encode (buffer_yuv, 240, 320 * 2);
  rle_encode (dct, rle, &size);
  rle_decode (rle, dct_two);
  restored = dct_decode (dct_two, 240, 320 * 2);
  unsigned char *pute = rgb422 (restored, 240, 320);
  printf ("%d\n", size);
  dumpToFile (pute, "srcd.rgb", 240 * 320 * 3);
}
