#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>


#include <glib.h>
#include <stdlib.h>
#include "huffman.h"

static void
free_tree (huffman_node * node)
{
  if (node) {
    if (node->left) {
      free_tree (node->left);
      node->left = 0;
    }
    if (node->right) {
      free_tree (node->right);
      node->right = 0;
    }
    if (node->left == 0 && node->right == 0)
      free (node);
  }
}

static gint
compare_ulong_function (gconstpointer a, gconstpointer b, gpointer user_data)
{
  if (((huffman_node *) a)->weight - ((huffman_node *) b)->weight)
    return ((huffman_node *) a)->weight - ((huffman_node *) b)->weight;
  else
    return ((huffman_node *) a)->symbol - ((huffman_node *) b)->symbol;
}

static GQueue *
build_priority_queue (unsigned char *const input_data,
    unsigned int const input_size)
{

  GQueue *priority_queue = g_queue_new ();
  double values[256] = { 0 };

  for (int i = 0; i < input_size; ++i)
    ++values[input_data[i]];

  for (int i = 0; i < 256; ++i) {
    if (values[i] > 0) {
      huffman_node *new_node = malloc (sizeof (huffman_node));
      new_node->symbol = (unsigned char) i;
      new_node->weight = values[i];
      new_node->left = 0;
      new_node->right = 0;
      g_queue_push_head (priority_queue, new_node);
    }
  }

  g_queue_sort (priority_queue, compare_ulong_function, 0);

  return priority_queue;
}

static GQueue *
restore_priority_queue (unsigned char *input_data,
    unsigned int const input_size)
{
  GQueue *priority_queue = g_queue_new ();

  for (int i = 0; i < input_size; ++i) {
    huffman_node *new_node = malloc (sizeof (huffman_node));
    huffman_freq *freq = (void *) input_data;

    new_node->symbol = freq->symbol;
    new_node->weight = freq->weight;
    new_node->left = 0;
    new_node->right = 0;
    g_queue_push_head (priority_queue, new_node);
    input_data += sizeof (huffman_freq);
  }

  g_queue_sort (priority_queue, compare_ulong_function, 0);

  return priority_queue;
}

/*
 *
 * 1.Create a leaf node for each symbol and add it to the priority queue.
 * 2.While there is more than one node in the queue:
 * 	-Remove the two nodes of highest priority (lowest probability) from the queue
 * 	-Create a new internal node with these two nodes as children and with probability equal to the sum of the two nodes' probabilities.
 * 	-Add the new node to the queue.
 * 3.The remaining node is the root node and the tree is complete.
 *
 */
static huffman_node *
build_huffman_tree (GQueue * priority_queue)
{
  while (g_queue_get_length (priority_queue) > 1) {
    huffman_node *node0 = g_queue_pop_head (priority_queue);
    huffman_node *node1 = g_queue_pop_head (priority_queue);
    huffman_node *internal_node = malloc (sizeof (huffman_node));

    internal_node->left = node0;
    internal_node->right = node1;
    internal_node->weight = node0->weight + node1->weight;
    internal_node->symbol = 0;
    g_queue_push_head (priority_queue, internal_node);
  }
  return g_queue_pop_head (priority_queue);
}

static void
build_code_table (huffman_node * root, huffman_code * code,
    unsigned char *current_code, int pos)
{
  if (root->left == 0 && root->right == 0) {
    memcpy (&code[root->symbol].code, current_code, sizeof (long));
    code[root->symbol].length = pos;
  } else {
    if (root->left != 0) {
      current_code[pos / 8] &= ~(0x80 >> (pos % 8));
      build_code_table (root->left, code, current_code, pos + 1);
    }
    if (root->right != 0) {
      current_code[pos / 8] |= 0x80 >> (pos % 8);
      build_code_table (root->right, code, current_code, pos + 1);
    }
  }
}

static int
calculate_compressed_size (GQueue * priority_queue, huffman_code * code_table)
{
  int res = 0;

  // Space for frequency list and the size of it plus size of original data
  // and the output data.
  for (unsigned int i = 0; i < g_queue_get_length (priority_queue); ++i) {
    huffman_node *node = g_queue_peek_nth (priority_queue, i);
    res += (code_table[node->symbol].length * node->weight);
  }
  if (res % 8)
    res += 8;
  res /= 8;
  res +=
      (sizeof (char) + sizeof (long)) * g_queue_get_length (priority_queue) +
      (3 * sizeof (int));
  return res;
}

void *
huffman_encode (unsigned char *const input_data, unsigned int const input_size)
{
  int compressed_size;
  unsigned char *output_data;
  GQueue *priority_queue = build_priority_queue (input_data, input_size);
  huffman_node *root = build_huffman_tree (g_queue_copy (priority_queue));
  huffman_code code_table[256] = { {0} };
  int size_of_priority_queue = g_queue_get_length (priority_queue);
  unsigned char current_code[32] = { 0 };
  void *res;

  build_code_table (root, code_table, current_code, 0);
  compressed_size = calculate_compressed_size (priority_queue, code_table);

  output_data = malloc (compressed_size);
  res = output_data;

  // Writing size of original file
  memcpy (output_data, &input_size, sizeof (int));
  output_data += sizeof (int);

  // Writing size of frequency table
  memcpy (output_data, &size_of_priority_queue, sizeof (int));
  output_data += sizeof (int);

  // Writing size of the output file
  memcpy (output_data, &compressed_size, sizeof (int));
  output_data += sizeof (int);

  // Writing frequency table
  for (unsigned int i = 0; i < size_of_priority_queue; ++i) {
    huffman_node *node = g_queue_peek_nth (priority_queue, i);
    huffman_freq *freq = (huffman_freq *) output_data;
    freq->symbol = node->symbol;
    freq->weight = node->weight;

    output_data += sizeof (huffman_freq);
  }

  // Writing compressed data
  for (int i = 0; i < input_size; ++i) {
    unsigned char byte_to_encode = input_data[i];
    huffman_code corresponding_code = code_table[byte_to_encode];
    static char byte = 0;
    int j = 0;
    unsigned char code_com = 0x80;

    while (corresponding_code.length != 0) {
      static unsigned char com = 0x80;

      while (com != 0 && code_com != 0 && corresponding_code.length != 0) {
        if (corresponding_code.code[j] & code_com)
          byte |= com;
        else
          byte &= ~com;
        --corresponding_code.length;
        com >>= 1;
        code_com >>= 1;
      }
      *output_data = byte;
      if (com == 0) {
        byte = 0x00;
        ++output_data;
        com = 0x80;
      }
      if (code_com == 0 && corresponding_code.length != 0) {
        ++j;
        code_com = 0x80;
      }
    }
  }

  free_tree (root);
  g_queue_free (priority_queue);
  return res;
}

unsigned char *
huffman_decode (unsigned char *input_data, unsigned int *output_size)
{
  // Reading size of output data
  unsigned char *output_data;
  int size_freq_table, compressed_size;
  GQueue *priority_queue;
  huffman_node *root, *curr;
  int j = 0;

  *output_size = *(int *) input_data;
  output_data = malloc (*output_size);
  input_data += sizeof (int);

  // Reading size of frequency table
  size_freq_table = *(int *) input_data;
  input_data += sizeof (int);

  // Reading size of compressed file
  compressed_size = *(int *) input_data;
  input_data += sizeof (int);

  priority_queue = restore_priority_queue (input_data, size_freq_table);
  input_data += size_freq_table * sizeof (huffman_freq);

  compressed_size -=
      (3 * sizeof (int)) + (size_freq_table * sizeof (huffman_freq));

  root = build_huffman_tree (g_queue_copy (priority_queue));
  curr = root;

  // Reading compressed data
  for (int i = 0; i < compressed_size; ++i) {
    unsigned char byte_to_decode = input_data[i];

    for (unsigned char com = 0x80; com != 0; com >>= 1) {
      if (byte_to_decode & com)
        curr = curr->right;
      else
        curr = curr->left;
      if (curr->left == 0 && curr->right == 0) {
        output_data[j] = curr->symbol;
        curr = root;
        ++j;
      }
    }
  }

  free_tree (root);
  g_queue_free (priority_queue);
  return output_data;
}

#ifdef ENABLE_MAIN

int
main (int argc, char *argv[])
{
  /*
     char* file_path = "bmp.bmp";
     int file_fd = open(file_path, O_RDONLY);
     struct stat file_stat;
     stat(file_path, &file_stat);
     void* mapped_file = mmap(0, file_stat.st_size, PROT_READ, MAP_SHARED, file_fd, 0);
     printf("%d\n", file_stat.st_size);

     int size;
     void* out = huffman_encode(mapped_file, file_stat.st_size);
     huffman_decode(out, &size);
   */
  char *j = "j'aime aller sur le bord de l'eau les jeudis ou les jours impairs";
  void *out;
  unsigned char *in = 0;
  int size = 0;
  out = huffman_encode (j, strlen (j));
  in = huffman_decode (out, &size);
  for (int i = 0; i < size; ++i) {
    printf ("%c", *(char *) in);
    ++in;
  }
  free (out);
  return 0;
}

#endif
