#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "rle.h"

int
main ()
{
  char *input =
      "WWWWWWWWWWWWBWWWWWWWWWWWWBBBWWWWWWWWWWWWWWWWWWWWWWWWBWWWWWWWWWWWWWW";
  unsigned char *compressed = malloc (strlen (input));
  unsigned char *restored = malloc (strlen (input));
  unsigned int size = strlen (input);

  rle_encode ((unsigned char *) input, compressed, &size);
  rle_decode (compressed, restored);
  printf ("Original : %s == size : %lu\n", input, strlen (input));
  printf ("Compressed : %s\n", compressed);
  printf ("Restored : %s\n", restored);

  return 0;
}
