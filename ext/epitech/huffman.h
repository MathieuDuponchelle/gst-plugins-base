#ifndef		__HUFFMAN_H
#define		__HUFFMAN_H


typedef struct node huffman_node;
typedef struct code huffman_code;
typedef struct freq huffman_freq;

struct node
{
	unsigned char symbol;
	unsigned long weight;
	huffman_node* left;
	huffman_node* right;
};

struct code
{
	unsigned char length;
	char code[32];
};

struct __attribute__((packed)) freq
{
	unsigned char symbol;
	unsigned long weight;
};

void* huffman_encode(unsigned char* const input_data, unsigned int const input_size);
unsigned char* huffman_decode(unsigned char* input_data, unsigned int *output_size);

#endif
