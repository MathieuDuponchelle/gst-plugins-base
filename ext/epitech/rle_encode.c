#include "rle.h"

/* Specification: encoded stream are unsigned bytes consisting of sequences.
 * First byte of each sequence is the length, followed by a number of bytes.
 * If length <=128, the next byte is to be repeated length times;
 * If length > 128, the next (length - 128) bytes are not repeated.
 * this is to improve efficiency for long non-repeating sequences.
 * This scheme can encode arbitrary byte values efficiently.
 * c.f. Adobe PDF spec RLE stream encoding (not exactly the same)
 */

void
rle_encode (unsigned char *in, unsigned char *out, unsigned int *length)
{
  unsigned char buf[256];
  int len = 0, repeat = 0, end = 0, c, i;
  unsigned char *c_out = out;

  int y = 0;
  while (y <= *length) {
    c = *in;
    end = !(y <= *length);
    in++;
    y++;
    if (!end) {
      buf[len++] = c;
      if (len <= 1)
        continue;
    }

    if (repeat) {
      if (buf[len - 1] != buf[len - 2])
        repeat = 0;
      if (!repeat || len == 129 || end) {
        *out = end ? len : len - 1;
        out++;
        *out = buf[0];
        out++;
        buf[0] = buf[len - 1];
        len = 1;
      }
    } else {
      if (buf[len - 1] == buf[len - 2]) {
        repeat = 1;
        if (len > 2) {
          *out = 128 + len - 2;
          out++;
          for (i = 0; i < len - 2; i++) {
            *out = buf[i];
            out++;
          }
          buf[0] = buf[1] = buf[len - 1];
          len = 2;
        }
        continue;
      }
      if (len == 128 || end) {
        *out = 128 + len - 2;
        out++;
        for (i = 0; i < len - 2; i++) {
          *out = buf[i];
          out++;
        }
        len = 0;
        repeat = 0;
      }
    }
  }
  *out = 0;
  out++;
  *length = out - c_out;
}
