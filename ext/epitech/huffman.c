#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <glib.h>
#include "bitop256.h"
#include "getopt.h"
#include "huffman.h"


/* allocation/deallocation routines */
static huffman_node_t *AllocHuffmanNode (int value);
static huffman_node_t *AllocHuffmanCompositeNode (huffman_node_t * left,
    huffman_node_t * right);
static void FreeHuffmanTree (huffman_node_t * ht);

/* build and display tree */
static huffman_node_t *GenerateTreeFromFile (huffman_node_t ** huffmanArray,
    unsigned char *buffer, count_t total);
static huffman_node_t *BuildHuffmanTree (huffman_node_t ** ht, int elements);
static int getHeaderSize (huffman_node_t * ht);
static unsigned char *EncodeFile (huffman_node_t * ht, unsigned char *buffer,
    count_t size, count_t * encoded_size);
static unsigned char *DecodeFile (huffman_node_t ** ht,
    unsigned char *encoded_buffer, count_t encoded_size,
    count_t * decoded_size);
static void MakeCodeList (huffman_node_t * ht, code_list_t * cl);

/* reading/writing tree to file */
static void WriteHeader (huffman_node_t * ht, unsigned char *buf);
static count_t ReadHeader (huffman_node_t ** ht, unsigned char *encoded_buffer,
    count_t encoded_size, count_t * total);

#ifdef ENABLE_MAIN

static void dumpToFile (unsigned char *buffer, const char *fName,
    count_t count);
static unsigned char *readFile (const char *name, count_t * count);

#endif

void *
huffman_encode (unsigned char *const input_data, unsigned int const input_size,
    unsigned int *output_size)
{
  huffman_node_t *huffmanTree;  /* root of huffman tree */
  void *encoded_buffer;
  huffman_node_t *huffmanArray[NUM_CHARS];
  huffmanTree = GenerateTreeFromFile (huffmanArray, input_data, input_size);
  encoded_buffer =
      EncodeFile (huffmanTree, input_data, input_size, output_size);
  return encoded_buffer;
}

void *
huffman_decode (unsigned char *input_data, unsigned int input_size,
    unsigned int *output_size)
{
  unsigned char *decoded_buffer;
  huffman_node_t *huffmanArray[NUM_CHARS];
  decoded_buffer =
      DecodeFile (huffmanArray, input_data, input_size, output_size);
  return decoded_buffer;
}

#ifdef ENABLE_MAIN
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
#endif

static huffman_node_t *
GenerateTreeFromFile (huffman_node_t ** huffmanArray,
    unsigned char *buffer, count_t total)
{
  huffman_node_t *huffmanTree;  /* root of huffman tree */
  int c;
  count_t i = 0;
  count_t count = 0;

  /* allocate array of leaves for all possible characters */
  for (c = 0; c < NUM_CHARS; c++) {
    huffmanArray[c] = AllocHuffmanNode (c);
  }

  /* count occurrence of each character */
  while (i < total) {
    c = buffer[i];
    if (count < COUNT_T_MAX) {
      count++;

      /* increment count for character and include in tree */
      huffmanArray[c]->count++; /* check for overflow */
      huffmanArray[c]->ignore = FALSE;
    }

    else {
      fprintf (stderr, "Number of characters in file is too large to count.\n");
      exit (EXIT_FAILURE);
    }
    i++;
  }

  /* put array of leaves into a huffman tree */
  huffmanTree = BuildHuffmanTree (huffmanArray, NUM_CHARS);
  return (huffmanTree);
}

static huffman_node_t *
AllocHuffmanNode (int value)
{
  huffman_node_t *ht;
  ht = (huffman_node_t *) (malloc (sizeof (huffman_node_t)));
  if (ht != NULL) {
    ht->value = value;
    ht->ignore = TRUE;          /* will be FALSE if one is found */

    /* at this point, the node is not part of a tree */
    ht->count = 0;
    ht->level = 0;
    ht->left = NULL;
    ht->right = NULL;
    ht->parent = NULL;
  }

  else {
    perror ("Allocate Node");
    exit (EXIT_FAILURE);
  }
  return ht;
}

static huffman_node_t *
AllocHuffmanCompositeNode (huffman_node_t * left, huffman_node_t * right)
{
  huffman_node_t *ht;
  ht = (huffman_node_t *) (malloc (sizeof (huffman_node_t)));
  if (ht != NULL) {
    ht->value = COMPOSITE_NODE; /* represents multiple chars */
    ht->ignore = FALSE;
    ht->count = left->count + right->count;     /* sum of children */
    ht->level = max (left->level, right->level) + 1;

    /* attach children */
    ht->left = left;
    ht->left->parent = ht;
    ht->right = right;
    ht->right->parent = ht;
    ht->parent = NULL;
  }

  else {
    perror ("Allocate Composite");
    exit (EXIT_FAILURE);
  }
  return ht;
}

static void
FreeHuffmanTree (huffman_node_t * ht)
{
  if (ht->left != NULL) {
    FreeHuffmanTree (ht->left);
  }
  if (ht->right != NULL) {
    FreeHuffmanTree (ht->right);
  }
  free (ht);
}

static int
FindMinimumCount (huffman_node_t ** ht, int elements)
{
  int i;                        /* array index */
  int currentIndex = NONE;      /* index with lowest count seen so far */
  int currentCount = INT_MAX;   /* lowest count seen so far */
  int currentLevel = INT_MAX;   /* level of lowest count seen so far */

  /* sequentially search array */
  for (i = 0; i < elements; i++) {

    /* check for lowest count (or equally as low, but not as deep) */
    if ((ht[i] != NULL) && (!ht[i]->ignore) &&
        (ht[i]->count < currentCount ||
            (ht[i]->count == currentCount && ht[i]->level < currentLevel))) {
      currentIndex = i;
      currentCount = ht[i]->count;
      currentLevel = ht[i]->level;
    }
  }
  return currentIndex;
}

static huffman_node_t *
BuildHuffmanTree (huffman_node_t ** ht, int elements)
{
  int min1, min2;               /* two nodes with the lowest count */

  /* keep looking until no more nodes can be found */
  for (;;) {

    /* find node with lowest count */
    min1 = FindMinimumCount (ht, elements);
    if (min1 == NONE) {

      /* no more nodes to combine */
      break;
    }
    ht[min1]->ignore = TRUE;    /* remove from consideration */

    /* find node with second lowest count */
    min2 = FindMinimumCount (ht, elements);
    if (min2 == NONE) {

      /* no more nodes to combine */
      break;
    }
    ht[min2]->ignore = TRUE;    /* remove from consideration */

    /* combine nodes into a tree */
    ht[min1] = AllocHuffmanCompositeNode (ht[min1], ht[min2]);
    ht[min2] = NULL;
  }
  return ht[min1];
}

static int
getPayloadSize (unsigned char *buffer, code_list_t codeList[], count_t count)
{
  int c, i, bitCount, j;
  char bitBuffer;
  int size;

  /* write encoded file 1 byte at a time */
  bitBuffer = 0;
  bitCount = 0;
  j = 0;
  size = 0;
  while (j < count) {
    c = (int) buffer[j];

    /* shift in bits */
    for (i = 0; i < codeList[c].codeLen; i++) {
      bitCount++;
      bitBuffer = (bitBuffer << 1) | (TestBit256 (codeList[c].code, i) == 1);
      if (bitCount == 8) {
        bitCount = 0;
        size += 1;
      }
    }
    j += 1;
  }

  /* now handle spare bits */
  if (bitCount != 0) {
    bitBuffer <<= 8 - bitCount;
    size += 1;
  }
  return size;
}

static unsigned char *
EncodeFile (huffman_node_t * ht, unsigned char *buffer, count_t count,
    count_t * encoded_size)
{
  code_list_t codeList[NUM_CHARS];      /* table for quick encode */
  int c, i, bitCount, j, k, header_size, payload_size;
  char bitBuffer;
  unsigned char *buf;
  MakeCodeList (ht, codeList);  /* convert code to easy to use list */
  header_size = getHeaderSize (ht);
  payload_size = getPayloadSize (buffer, codeList, count);
  buf = g_malloc (sizeof (unsigned char) * (header_size + payload_size));
  *encoded_size = header_size + payload_size;
  WriteHeader (ht, buf);

  /* write encoded file 1 byte at a time */
  bitBuffer = 0;
  bitCount = 0;
  j = 0;
  k = header_size;
  while (j < count) {
    c = (int) buffer[j];

    /* shift in bits */
    for (i = 0; i < codeList[c].codeLen; i++) {
      bitCount++;
      bitBuffer = (bitBuffer << 1) | (TestBit256 (codeList[c].code, i) == 1);
      if (bitCount == 8) {

        /* we have a byte in the buffer */
        buf[k] = bitBuffer;
        bitCount = 0;
        k += 1;
      }
    }
    j += 1;
  }

  /* now handle spare bits */
  if (bitCount != 0) {
    bitBuffer <<= 8 - bitCount;
    buf[k] = bitBuffer;
    k += 1;
  }
  return buf;
}

static void
MakeCodeList (huffman_node_t * ht, code_list_t * cl)
{
  code_t code[32];
  byte_t depth = 0;
  ClearAll256 (code);
  for (;;) {

    /* follow this branch all the way left */
    while (ht->left != NULL) {
      LeftShift256 (code, 1);
      ht = ht->left;
      depth++;
    }
    if (ht->value != COMPOSITE_NODE) {

      /* enter results in list */
      cl[ht->value].codeLen = depth;
      Copy256 (cl[ht->value].code, code);

      /* now left justify code */
      LeftShift256 (cl[ht->value].code, 256 - depth);
    }
    while (ht->parent != NULL) {
      if (ht != ht->parent->right) {

        /* try the parent's right */
        code[31] |= 0x01;
        ht = ht->parent->right;
        break;
      }

      else {

        /* parent's right tried, go up one level yet */
        depth--;
        RightShift256 (code, 1);
        ht = ht->parent;
      }
    }
    if (ht->parent == NULL) {

      /* we're at the top with nowhere to go */
      break;
    }
  }
}

static int
getHeaderSize (huffman_node_t * ht)
{
  int i;
  int j = 0;
  for (;;) {

    /* follow this branch all the way left */
    while (ht->left != NULL) {
      ht = ht->left;
    }
    if (ht->value != COMPOSITE_NODE) {
      j += 1;
      for (i = 0; i < sizeof (count_t); i++) {
        j += 1;
      }
    }
    while (ht->parent != NULL) {
      if (ht != ht->parent->right) {
        ht = ht->parent->right;
        break;
      }

      else {

        /* parent's right tried, go up one level yet */
        ht = ht->parent;
      }
    }
    if (ht->parent == NULL) {

      /* we're at the top with nowhere to go */
      break;
    }
  }

  /* now write end of table char 0 count 0 */
  j += 1;
  for (i = 0; i < sizeof (count_t); i++) {
    j += 1;
  }
  return j;
}

static void
WriteHeader (huffman_node_t * ht, unsigned char *buf)
{
  count_byte_t byteUnion;
  int i;
  int j = 0;
  for (;;) {

    /* follow this branch all the way left */
    while (ht->left != NULL) {
      ht = ht->left;
    }
    if (ht->value != COMPOSITE_NODE) {

      /* write symbol and count to header */
      buf[j] = (unsigned char) ht->value;
      j += 1;
      byteUnion.count = ht->count;
      for (i = 0; i < sizeof (count_t); i++) {
        buf[j] = (unsigned char) byteUnion.byte[i];
        j += 1;
    }}
    while (ht->parent != NULL) {
      if (ht != ht->parent->right) {
        ht = ht->parent->right;
        break;
      }

      else {

        /* parent's right tried, go up one level yet */
        ht = ht->parent;
      }
    }
    if (ht->parent == NULL) {

      /* we're at the top with nowhere to go */
      break;
    }
  }

  /* now write end of table char 0 count 0 */
  buf[j] = (unsigned char) 0;
  j += 1;
  for (i = 0; i < sizeof (count_t); i++) {
    buf[j] = (unsigned char) 0;
    j += 1;
}} unsigned char *

DecodeFile (huffman_node_t ** ht, unsigned char *encoded_buffer,
    count_t encoded_size, count_t * decoded_size)
{
  huffman_node_t *huffmanTree, *currentNode;
  int i, c, j, k;
  count_t total = 0;
  count_t total_count = 0;
  unsigned char *buffer = NULL;

  /* allocate array of leaves for all possible characters */
  for (i = 0; i < NUM_CHARS; i++) {
    ht[i] = AllocHuffmanNode (i);
  }

  /* populate leaves with frequency information from file header */
  k = ReadHeader (ht, encoded_buffer, encoded_size, &total);
  total_count = total;
  buffer = g_malloc (sizeof (unsigned char) * total_count);

  /* put array of leaves into a huffman tree */
  huffmanTree = BuildHuffmanTree (ht, NUM_CHARS);

  /* now we should have a tree that matches the tree used on the encode */
  currentNode = huffmanTree;
  j = 0;

  /* handle one symbol codes */
  if (currentNode->value != COMPOSITE_NODE) {
    k = encoded_size;

    /* now just write out number of required symbols */
    while (total) {
      buffer[j] = (unsigned char) (currentNode->value);

      /*fputc((currentNode->value, fpOut); */
      total--;
      j++;
  }}
  while (k < encoded_size) {
    c = encoded_buffer[k];
    k += 1;

    /* traverse the tree finding matches for our characters */
    for (i = 0; i < 8; i++) {
      if (c & 0x80) {
        currentNode = currentNode->right;
      }

      else {
        currentNode = currentNode->left;
      }
      if (currentNode->value != COMPOSITE_NODE) {

        /* we've found a character */
        /*fputc(currentNode->value, fpOut); */
        buffer[j] = (unsigned char) (currentNode->value);
        currentNode = huffmanTree;
        total--;
        j++;
        if (total == 0) {

          /* we've just written the last character */
          break;
        }
      }
      c <<= 1;
    }
  }
  *decoded_size = total_count;
  FreeHuffmanTree (huffmanTree);        /* free allocated memory */
  return buffer;
}

static count_t
ReadHeader (huffman_node_t ** ht, unsigned char *encoded_buffer,
    count_t encoded_size, count_t * total)
{
  count_byte_t byteUnion;
  int c;
  int i;
  int j = 0;
  while (j < encoded_size) {
    c = encoded_buffer[j];
    j += 1;
    for (i = 0; i < sizeof (count_t); i++) {
      byteUnion.byte[i] = (byte_t) encoded_buffer[j];
      j += 1;
    }
    if ((byteUnion.count == 0) && (c == 0)) {

      /* we just read end of table marker */
      break;
    }
    ht[c]->count = byteUnion.count;
    ht[c]->ignore = FALSE;
    (*total) += byteUnion.count;
  }
  return j;
}

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
#endif
