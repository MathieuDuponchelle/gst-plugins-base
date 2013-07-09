#ifndef		__HUFFMAN_H
#define		__HUFFMAN_H

#define NONE    -1

#define COUNT_T_MAX     UINT_MAX        /* based on count_t being unsigned int */

#define COMPOSITE_NODE      -1  /* node represents multiple characters */
#define NUM_CHARS           256 /* 256 possible 1-byte symbols */

#define max(a, b) ((a)>(b)?(a):(b))

#if (UCHAR_MAX != 0xFF)
#error This program expects unsigned char to be 1 byte
#endif /*  */

#if (USHRT_MAX != 0xFFFF)
#error This program expects unsigned short to be 2 bytes
#endif /*  */

#if (UINT_MAX != 0xFFFFFFFF)
#error This program expects unsigned int to be 4 bytes
#endif /*  */

/* system dependent types */
typedef unsigned char byte_t;   /* unsigned 8 bit */
typedef unsigned char code_t;   /* unsigned 8 bit for character codes */
typedef unsigned int count_t;   /* unsigned 32 bit for character counts */

/* breaks count_t into array of byte_t */
typedef union count_byte_t
{
  count_t count;
  byte_t byte[sizeof (count_t)];
}
count_byte_t;
typedef struct huffman_node_t
{
  int value;                    /* character(s) represented by this entry */
  count_t count;                /* number of occurrences of value (probability) */
  char ignore;                  /* TRUE -> already handled or no need to handle */
  int level;                    /* depth in tree (root is 0) */

    /***********************************************************************
    *  pointer to children and parent.
    *  NOTE: parent is only useful if non-recursive methods are used to
    *        search the huffman tree.
    ***********************************************************************/
  struct huffman_node_t *left, *right, *parent;
} huffman_node_t;
typedef struct code_list_t
{
  byte_t codeLen;               /* number of bits used in code (1 - 255) */
  code_t code[32];              /* code used for symbol (left justified) */
} code_list_t;
typedef enum
{ BUILD_TREE, COMPRESS, DECOMPRESS
} MODES;


void* huffman_encode(unsigned char* const input_data, unsigned int const input_size, unsigned int *output_size);
void* huffman_decode(unsigned char* input_data, unsigned int input_size, unsigned int *output_size);

#endif
