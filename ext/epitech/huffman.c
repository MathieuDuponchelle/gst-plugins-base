#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <glib.h>
#include "bitop256.h"
#include "getopt.h"
#include "huffman.h"


static huffman_node_t *alloc_huffman_node (int value);
static huffman_node_t *alloc_huffman_composite_node (huffman_node_t * left,
    huffman_node_t * right);
static void free_huffman_tree (huffman_node_t * ht);
static huffman_node_t *generate_tree_from_file (huffman_node_t ** huffmanArray,
    unsigned char *buffer, count_t total);
static huffman_node_t *build_huffman_tree (huffman_node_t ** ht, int elements);
static int get_header_size (huffman_node_t * ht);
static unsigned char *encode_file (huffman_node_t * ht, unsigned char *buffer,
    count_t size, count_t * encoded_size);
static unsigned char *decode_file (huffman_node_t ** ht,
    unsigned char *encoded_buffer, count_t encoded_size,
    count_t * decoded_size);
static void make_code_list (huffman_node_t * ht, code_list_t * cl);
static void write_header (huffman_node_t * ht, unsigned char *buf);
static count_t read_header (huffman_node_t ** ht, unsigned char *encoded_buffer,
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
  huffman_node_t *huffmanTree;
  void *encoded_buffer;
  huffman_node_t *huffmanArray[NUM_CHARS];

  huffmanTree = generate_tree_from_file (huffmanArray, input_data, input_size);
  encoded_buffer =
      encode_file (huffmanTree, input_data, input_size, output_size);

  return encoded_buffer;
}

void *
huffman_decode (unsigned char *input_data, unsigned int input_size,
    unsigned int *output_size)
{
  unsigned char *decoded_buffer;
  huffman_node_t *huffmanArray[NUM_CHARS];

  decoded_buffer =
      decode_file (huffmanArray, input_data, input_size, output_size);

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
generate_tree_from_file (huffman_node_t ** huffmanArray,
    unsigned char *buffer, count_t total)
{
  huffman_node_t *huffmanTree;
  int c;
  count_t i = 0;
  count_t count = 0;

  for (c = 0; c < NUM_CHARS; c++) {
    huffmanArray[c] = alloc_huffman_node (c);
  }

  while (i < total) {
    c = buffer[i];

    if (count < COUNT_T_MAX) {
      count++;
      huffmanArray[c]->count++;
      huffmanArray[c]->ignore = FALSE;
    }

    else {
      fprintf (stderr, "Number of characters in file is too large to count.\n");
      exit (EXIT_FAILURE);
    }
    i++;
  }

  huffmanTree = build_huffman_tree (huffmanArray, NUM_CHARS);

  return (huffmanTree);
}

static huffman_node_t *
alloc_huffman_node (int value)
{
  huffman_node_t *ht;
  ht = (huffman_node_t *) (malloc (sizeof (huffman_node_t)));

  if (ht != NULL) {
    ht->value = value;
    ht->ignore = TRUE;

    ht->count = 0;
    ht->level = 0;
    ht->left = NULL;
    ht->right = NULL;
    ht->parent = NULL;
  } else {
    perror ("Allocate Node");
    exit (EXIT_FAILURE);
  }

  return ht;
}

static huffman_node_t *
alloc_huffman_composite_node (huffman_node_t * left, huffman_node_t * right)
{
  huffman_node_t *ht;
  ht = (huffman_node_t *) (malloc (sizeof (huffman_node_t)));

  if (ht != NULL) {
    ht->value = COMPOSITE_NODE;
    ht->ignore = FALSE;
    ht->count = left->count + right->count;
    ht->level = max (left->level, right->level) + 1;

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
free_huffman_tree (huffman_node_t * ht)
{
  if (ht->left != NULL)
    free_huffman_tree (ht->left);
  if (ht->right != NULL)
    free_huffman_tree (ht->right);
  free (ht);
}

static int
FindMinimumCount (huffman_node_t ** ht, int elements)
{
  int i;
  int currentIndex = NONE;
  int currentCount = INT_MAX;
  int currentLevel = INT_MAX;

  for (i = 0; i < elements; i++) {
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
build_huffman_tree (huffman_node_t ** ht, int elements)
{
  int min1, min2;

  for (;;) {
    min1 = FindMinimumCount (ht, elements);
    if (min1 == NONE)
      break;

    ht[min1]->ignore = TRUE;
    min2 = FindMinimumCount (ht, elements);
    if (min2 == NONE)
      break;

    ht[min2]->ignore = TRUE;
    ht[min1] = alloc_huffman_composite_node (ht[min1], ht[min2]);
    ht[min2] = NULL;
  }

  return ht[min1];
}

static int
getPayloadSize (unsigned char *buffer, code_list_t codeList[], count_t count)
{
  int c, i, bitCount, j;
  char bitBuffer = 0;
  int size = 0;

  bitCount = 0;
  j = 0;

  while (j < count) {
    c = (int) buffer[j];

    for (i = 0; i < codeList[c].codeLen; i++) {
      bitCount++;
      bitBuffer = (bitBuffer << 1) | (TestBit256 (codeList[c].code, i) == 1);
      if (bitCount == 8) {
        bitCount = 0;
        size++;
      }
    }
    j++;
  }

  if (bitCount != 0) {
    bitBuffer <<= 8 - bitCount;
    size++;
  }

  return size;
}

static unsigned char *
encode_file (huffman_node_t * ht, unsigned char *buffer, count_t count,
    count_t * encoded_size)
{
  code_list_t codeList[NUM_CHARS];
  int c, i, bitCount, j, k, header_size, payload_size;
  char bitBuffer;
  unsigned char *buf;
  make_code_list (ht, codeList);
  header_size = get_header_size (ht);
  payload_size = getPayloadSize (buffer, codeList, count);
  buf = g_malloc (sizeof (unsigned char) * (header_size + payload_size));
  *encoded_size = header_size + payload_size;
  write_header (ht, buf);

  bitBuffer = 0;
  bitCount = 0;
  j = 0;
  k = header_size;
  while (j < count) {
    c = (int) buffer[j];

    for (i = 0; i < codeList[c].codeLen; i++) {
      bitCount++;
      bitBuffer = (bitBuffer << 1) | (TestBit256 (codeList[c].code, i) == 1);
      if (bitCount == 8) {
        buf[k] = bitBuffer;
        bitCount = 0;
        k++;
      }
    }
    j++;
  }

  if (bitCount != 0) {
    bitBuffer <<= 8 - bitCount;
    buf[k] = bitBuffer;
    k++;
  }

  return buf;
}

static void
make_code_list (huffman_node_t * ht, code_list_t * cl)
{
  code_t code[32];
  byte_t depth = 0;

  ClearAll256 (code);
  for (;;) {
    while (ht->left != NULL) {
      LeftShift256 (code, 1);
      ht = ht->left;
      depth++;
    }
    if (ht->value != COMPOSITE_NODE) {
      cl[ht->value].codeLen = depth;
      Copy256 (cl[ht->value].code, code);
      LeftShift256 (cl[ht->value].code, 256 - depth);
    }
    while (ht->parent != NULL) {
      if (ht != ht->parent->right) {
        code[31] |= 0x01;
        ht = ht->parent->right;
        break;
      } else {
        depth--;
        RightShift256 (code, 1);
        ht = ht->parent;
      }
    }
    if (ht->parent == NULL)
      break;
  }
}

static int
get_header_size (huffman_node_t * ht)
{
  int i;
  int j = 0;

  for (;;) {
    while (ht->left != NULL)
      ht = ht->left;

    if (ht->value != COMPOSITE_NODE) {
      j++;
      for (i = 0; i < sizeof (count_t); i++) {
        j++;
      }
    }

    while (ht->parent != NULL) {
      if (ht != ht->parent->right) {
        ht = ht->parent->right;
        break;
      } else {
        ht = ht->parent;
      }
    }
    if (ht->parent == NULL)
      break;
  }

  j++;
  for (i = 0; i < sizeof (count_t); i++)
    j++;

  return j;
}

static void
write_header (huffman_node_t * ht, unsigned char *buf)
{
  count_byte_t byteUnion;
  int i;
  int j = 0;

  for (;;) {
    while (ht->left != NULL)
      ht = ht->left;

    if (ht->value != COMPOSITE_NODE) {
      buf[j] = (unsigned char) ht->value;
      j++;
      byteUnion.count = ht->count;
      for (i = 0; i < sizeof (count_t); i++) {
        buf[j] = (unsigned char) byteUnion.byte[i];
        j++;
      }
    }
    while (ht->parent != NULL) {
      if (ht != ht->parent->right) {
        ht = ht->parent->right;
        break;
      } else {
        ht = ht->parent;
      }
    }
    if (ht->parent == NULL)
      break;
  }

  buf[j] = (unsigned char) 0;
  j++;
  for (i = 0; i < sizeof (count_t); i++) {
    buf[j] = (unsigned char) 0;
    j++;
  }
}

static unsigned char *
decode_file (huffman_node_t ** ht, unsigned char *encoded_buffer,
    count_t encoded_size, count_t * decoded_size)
{
  huffman_node_t *huffmanTree, *currentNode;
  int i, c, j, k;
  count_t total = 0;
  count_t total_count = 0;
  unsigned char *buffer = NULL;

  for (i = 0; i < NUM_CHARS; i++)
    ht[i] = alloc_huffman_node (i);

  k = read_header (ht, encoded_buffer, encoded_size, &total);
  total_count = total;
  buffer = g_malloc (sizeof (unsigned char) * total_count);
  huffmanTree = build_huffman_tree (ht, NUM_CHARS);
  currentNode = huffmanTree;
  j = 0;

  if (currentNode->value != COMPOSITE_NODE) {
    k = encoded_size;

    while (total) {
      buffer[j] = (unsigned char) (currentNode->value);

      total--;
      j++;
  }}
  while (k < encoded_size) {
    c = encoded_buffer[k];
    k++;

    for (i = 0; i < 8; i++) {
      if (c & 0x80)
        currentNode = currentNode->right;
      else
        currentNode = currentNode->left;

      if (currentNode->value != COMPOSITE_NODE) {
        buffer[j] = (unsigned char) (currentNode->value);
        currentNode = huffmanTree;
        total--;
        j++;
        if (total == 0)
          break;
      }
      c <<= 1;
    }
  }
  *decoded_size = total_count;
  free_huffman_tree (huffmanTree);

  return buffer;
}

static count_t
read_header (huffman_node_t ** ht, unsigned char *encoded_buffer,
    count_t encoded_size, count_t * total)
{
  count_byte_t byteUnion;
  int c;
  int i;
  int j = 0;

  while (j < encoded_size) {
    c = encoded_buffer[j];
    j++;
    for (i = 0; i < sizeof (count_t); i++) {
      byteUnion.byte[i] = (byte_t) encoded_buffer[j];
      j++;
    }
    if ((byteUnion.count == 0) && (c == 0))
      break;
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
