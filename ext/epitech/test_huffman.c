#include <stdio.h>
#include <stdlib.h>
#include "huffman.h"


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
  (void) fread (buffer, fileLen, 1, file);
  fclose (file);
  *count = fileLen;

  return buffer;
}

int
main (int argc, char *argv[])
{
  count_t count = 0;
  unsigned char *buffer = readFile ("src.rgb", &count);
  unsigned char *encoded_buffer;
  count_t encoded_size;
  unsigned char *decoded_buffer;
  count_t decoded_size;

  encoded_buffer = huffman_encode (buffer, count, &encoded_size);
  decoded_buffer = huffman_decode (encoded_buffer, encoded_size, &decoded_size);
  dumpToFile (decoded_buffer, "srcd.rgb", decoded_size);

  return (0);
}
