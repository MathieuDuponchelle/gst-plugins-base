#include <limits.h>
#include <string.h>
/* use preprocessor to verify type length */
#if (UCHAR_MAX != 0xFF)
#error This program expects unsigned char to be 1 byte
#endif /*  */

#define NUM_BITS    256
#define NUM_BYTES   32          /* 32 x 8 = 256 */
#define LAST_BYTE   31          /* bytes are numbered 0 .. 31 */

#include "bitop256.h"
void
ClearAll256 (unsigned char *bits)
{
  memset ((void *) bits, 0, NUM_BYTES);
} int

TestBit256 (const unsigned char *bits, unsigned char bit)
{
  int byte;
  unsigned char mask;
  byte = bit / 8;               /* target byte */
  bit = bit % 8;                /* target bit in byte */
  mask = (0x80 >> bit);
  return ((bits[byte] & mask) != 0);
}

void
Copy256 (unsigned char *dest, const unsigned char *src)
{
  memcpy ((void *) dest, (void *) src, NUM_BYTES);
} void

LeftShift256 (unsigned char *bits, int shifts)
{
  int i;
  unsigned int overflow;
  int bytes = shifts / 8;       /* number of whole byte shifts */
  shifts = shifts % 8;          /* number of bit shifts remaining */

  /* first handle big jumps of bytes */
  if (bytes > 0) {
    for (i = 0; (i + bytes) < NUM_BYTES; i++) {
      bits[i] = bits[i + bytes];
    }

    /* now zero out new bytes on the right */
    for (i = NUM_BYTES; bytes > 0; bytes--) {
      bits[i - bytes] = 0;
    }
  }

  /* now handle the remaining shifts (no more than 7 bits) */
  if (shifts > 0) {
    bits[0] <<= shifts;
    for (i = 1; i < NUM_BYTES; i++) {
      overflow = bits[i];
      overflow <<= shifts;
      bits[i] = (unsigned char) overflow;

      /* handle shifts across byte bounds */
      if (overflow & 0xFF00) {
        bits[i - 1] |= (unsigned char) (overflow >> 8);
      }
  }}
} void

RightShift256 (unsigned char *bits, int shifts)
{
  int i;
  unsigned int overflow;
  int bytes = shifts / 8;       /* number of whole byte shifts */
  shifts = shifts % 8;          /* number of bit shifts remaining */

  /* first handle big jumps of bytes */
  if (bytes > 0) {
    for (i = LAST_BYTE; (i - bytes) >= 0; i--) {
      bits[i] = bits[i - bytes];
    }

    /* now zero out new bytes on the right */
    for (; bytes > 0; bytes--) {
      bits[bytes - 1] = 0;
    }
  }

  /* now handle the remaining shifts (no more than 7 bits) */
  if (shifts > 0) {
    bits[LAST_BYTE] >>= shifts;
    for (i = LAST_BYTE - 1; i >= 0; i--) {
      overflow = bits[i];
      overflow <<= (8 - shifts);
      bits[i] = (unsigned char) (overflow >> 8);

      /* handle shifts across byte bounds */
      if (overflow & 0xFF) {
        bits[i + 1] |= (unsigned char) overflow;
      }
  }}
}
