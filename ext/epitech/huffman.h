#ifndef		__HUFFMAN_H
#define		__HUFFMAN_H

void* huffman_encode(unsigned char* const input_data, unsigned int const input_size, unsigned int *output_size);
void* huffman_decode(unsigned char* input_data, unsigned int input_size, unsigned int *output_size);

#endif
