#include "rle.h"

void
rle_decode (unsigned char *in, unsigned char *out)
{
  int c, i, cnt;

  c = *in;
  while (c != 0) {
    c = *in;
    in++;
    if (c > 128) {
      cnt = c - 128;
      for (i = 0; i < cnt; i++) {
        *out = *in;
        in++;
        out++;
      }
    } else {
      cnt = c;
      c = *in;
      in++;
      for (i = 0; i < cnt; i++) {
        *out = c;
        out++;
      }
    }
  }
}
