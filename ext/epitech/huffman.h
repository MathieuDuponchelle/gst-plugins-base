#ifndef		__HUFFMAN_H
#define		__HUFFMAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>

#define CODE_MAX_LEN 32
//#define BUF_LEN 262144
#define LOL_HARD_MALLOC 512

/* Definition des structures */
struct arbre
{
  short branche0;
  short branche1;
};

struct arbre_data
{
  unsigned long frequence;
  short index_suivant;
};

struct dictionnaire
{
  unsigned char taille;
  char code[CODE_MAX_LEN];
};

/* Prototypes des fonctions */
short huffman_calculer_frequences (unsigned char *, unsigned long *,
    unsigned short *, unsigned int size);
short huffman_lire_frequences (unsigned char *);
short huffman_creer_arbre (short);
void huffman_creer_dictionnaire (unsigned char *, short, short);
void *huffman_compacter (unsigned char *, unsigned int size);
void *huffman_decompacter (unsigned char *, unsigned int *size);
#endif
