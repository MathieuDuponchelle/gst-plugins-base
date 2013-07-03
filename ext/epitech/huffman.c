#include "huffman.h"

/* Variables globales */
struct arbre_data *arbre_d;
struct arbre *arbre;
struct dictionnaire *dico;

/*
 * Calcule la fréquence de chaque caractère dans un fichier et trie la liste de structures
 * par fréquences croissantes
 * *src : pointeur sur le fichier
 * *nbre_octets : pointeur sur une variable recevant la taille du fichier
 * *nbre_ascii : pointeur sur une variable recevant le nombre de caractères
 * différents présents dans le fichier source
 * Retourne l'index de la première structure dans la liste triée
 * */
short
huffman_calculer_frequences (unsigned char *src, unsigned long *nbre_octets,
    unsigned short *nbre_ascii, unsigned int size)
{
  int i;
  //unsigned char buffer[BUF_LEN];
  short index_frequence = 0, index_precedent = -1;

  int continuer = 1;
  short c1, c2;

  *nbre_octets = 0;
  *nbre_ascii = 0;

  memset (arbre_d, 0, LOL_HARD_MALLOC * sizeof (struct arbre_data));

  /*Lecture dans le fichier */
  //REPLACED
  *nbre_octets = size;
  for (i = 0; i < size; i++)
    arbre_d[src[i]].frequence++;
  /*
     while ((r=fread(buffer, 1, BUF_LEN, src))>0)
     {
     *nbre_octets+=r;
     for(i=0; i<r; i++)
     arbre_d[buffer[i]].frequence++;
     }
   */

  /* Chainage des structures avec une fréquence supérieure à 0 */
  for (i = 0; i < 256; i++)
    if (arbre_d[i].frequence > 0) {
      (*nbre_ascii)++;
      if (index_precedent == -1)
        index_frequence = i;
      else
        arbre_d[index_precedent].index_suivant = i;
      index_precedent = i;
    }
  if (index_precedent == -1)
    index_frequence = -1;
  else
    arbre_d[index_precedent].index_suivant = -1;

  /* Tri des structures (bubble sort) */
  while (continuer) {
    c1 = index_frequence;
    continuer = 0;
    index_precedent = -1;
    while (c1 != -1) {
      if ((c2 = arbre_d[c1].index_suivant) != -1) {
        if (arbre_d[c1].frequence > arbre_d[c2].frequence) {
          continuer = 1;
          if (index_precedent == -1)
            index_frequence = c2;
          else
            arbre_d[index_precedent].index_suivant = c2;
          arbre_d[c1].index_suivant = arbre_d[c2].index_suivant;
          arbre_d[c2].index_suivant = c1;
        }
        index_precedent = c1;
        c1 = c2;
      } else
        c1 = c2;
    }
  }

  /* On retourne l'index de la première structure */
  return index_frequence;
}

/*
 * Lit les fréquences de chaque caractère, inscrites soit dans le fichier
 * compressé, soit dans un fichier de fréquences spécial
 * */
short
huffman_lire_frequences (unsigned char *frq)
{
  unsigned short nbre_ascii;
  unsigned char i;
  short index_frequence = -1, index_precedent = -1;

  /* Lecture de la taille de la table des fréquences */
  memcpy (&nbre_ascii, frq, 2);
  frq += 2;
  //REPLACED
  //fread(&nbre_ascii, 2, 1, frq);

  /* Lecture de la table des fréquences */
  while (nbre_ascii > 0) {
    /* Lecture du caractère en cours */
    //REPLACED
    //fread(&i, 1, 1, frq);
    memcpy (&i, frq, 1);
    frq++;
    /* Lecture de la fréquence du caractère en cours */
    //REPLACED
    //fread((char *)&arbre_d[i].frequence, 4, 1, frq);
    memcpy (&arbre_d[i].frequence, frq, 4);
    frq += 4;
    /* Chainage de la structure */
    if (index_frequence == -1)
      index_frequence = i;
    else
      arbre_d[index_precedent].index_suivant = i;
    index_precedent = i;
    nbre_ascii--;
  }
  if (index_precedent == -1)
    return -1;
  arbre_d[index_precedent].index_suivant = -1;
  return index_frequence;
}

/*
 * Crée un arbre à partir d'une liste de fréquences
 * index_fréquence : idnex de la première structure dans la liste *arbre_d
 * Retourne l'index de la racine de l'arbre dans la liste *arbre
 * */
short
huffman_creer_arbre (short index_frequence)
{
  short i, j, j_save;
  unsigned long somme_frequence;
  short nbre_noeuds = 256;
  char struct_inseree = 0;

  /* Les structures 0 à 255 correspondent aux caractères, ce sont des terminaisons => -1 */
  for (j = 0; j < 256; j++)
    arbre[j].branche0 = arbre[j].branche1 = -1;

  /* Création de l'arbre :
   *    La mise en commun les deux fréquences les plus faibles crée un nouveau noeud avec une frequence
   *       égale a la somme des deux fréquences.
   *          Il s'agit ensuite d'insérer cette nouvelle structure dans la liste triée */
  i = index_frequence;
  while (i != -1) {
    if (arbre_d[i].index_suivant == -1) {
      /*printf("Arbre cree\n"); */
      break;
    }
    /*printf("%d\n", arbre_d[i].frequence);
     *       printf("%d\n", arbre_d[arbre_d[i].index_suivant].frequence); */
    somme_frequence =
        arbre_d[i].frequence + arbre_d[arbre_d[i].index_suivant].frequence;
    /*printf("Nouveau noeud : %d (%d) et %d (%d) => %d\n", i, arbre_d[i].frequence, 
     *          arbre_d[i].index_suivant, arbre_d[arbre_d[i].index_suivant].frequence, 
     *                   somme_frequence);*/
    arbre_d[nbre_noeuds].frequence = somme_frequence;
    arbre[nbre_noeuds].branche0 = arbre_d[i].index_suivant;
    arbre[nbre_noeuds].branche1 = i;
    /* Insertion du nouveau noeud dans la liste triée */
    j_save = -1;
    struct_inseree = 0;
    j = i;
    while (j != -1 && struct_inseree == 0) {
      if (arbre_d[j].frequence >= somme_frequence) {
        if (j_save != -1)
          arbre_d[j_save].index_suivant = nbre_noeuds;
        arbre_d[nbre_noeuds].index_suivant = j;
        /*printf("Insertion du nouveau noeud : entre %d et %d\n", 
         *                j_save==-1?-1:arbre_d[j_save].frequence, arbre_d[j].frequence);*/
        struct_inseree = 1;
      }
      j_save = j;
      j = arbre_d[j].index_suivant;
    }
    /* Insertion du nouveau noeud a la fin */
    if (struct_inseree == 0) {
      arbre_d[j_save].index_suivant = nbre_noeuds;
      arbre_d[nbre_noeuds].index_suivant = -1;
      /*printf("Insertion du nouveau noeud à la fin : %d\n", arbre_d[j_save].frequence); */
    }
    nbre_noeuds++;
    i = arbre_d[i].index_suivant;
    i = arbre_d[i].index_suivant;
  }
  /* On retourne l'index du noeud racine */
  return nbre_noeuds - 1;
}

/*
 * Procédure récursive qui crée un dictionnaire (correspondance entre la valeur ascii
 * d'un caractère et son codage obtenu avec la compression huffman) à partir d'un arbre
 * *code : pointeur sur une zone mémoire de taille CODE_MAX_LEN recevant
 * temporairement le code, au fur et à mesure de la progression dans l'arbre
 * index : position dans l'arbre (index de la structure courante)
 * pos : nombre de bits deja inscrits dans *code
 * */
void
huffman_creer_dictionnaire (unsigned char *code, short index, short pos)
{
  /* On a atteint une terminaison de l'arbre : c'est un caractère */
  if ((arbre[index].branche0 == -1) && (arbre[index].branche1 == -1)) {
    /* Copie du code dans le dictionnaire */
    memcpy (dico[index].code, code, CODE_MAX_LEN);
    /*printf("%c: %x - %d\n", index, code[0], pos); */
    /* taille du code en bits */
    dico[index].taille = (unsigned char) pos;
  }
  /* le noeud possède d'autres branches : on continue à les suivre */
  else {
    /* On suit la branche ajoutant un bit valant 0 */
    code[pos / 8] &= ~(0x80 >> (pos % 8));
    /* Le "(short)" devant "(pos+1)", c'est juste pour empecher VC++
     *       de chipoter (Warning : integral size mismatch in argument : 
     *             conversion supplied) */
    huffman_creer_dictionnaire (code, arbre[index].branche0, (short) (pos + 1));
    /* On suit la branche ajoutant un bit valant 1 */
    code[pos / 8] |= 0x80 >> (pos % 8);
    huffman_creer_dictionnaire (code, arbre[index].branche1, (short) (pos + 1));
  }
}

/*
 * Compresse le fichier src.
 * Le résulat se trouve dans le fichier dst
 * Si frq est différent de NULL, il est utilsé pour lire la table des fréquences
 * nécessaire à la construction de l'arbre
 * */
void *
huffman_compacter (unsigned char *src, unsigned int size,
    unsigned int *output_size)
{
  int i, octet_r, bit_r, bit_count, bit_w;
  unsigned long nbre_octets;
  short index_frequence;
  short racine_arbre;
  unsigned short nbre_ascii = 0;
  unsigned char code[CODE_MAX_LEN];
  //unsigned char buffer[BUF_LEN];
  //ADDED
  unsigned char *dst = 0;
  unsigned char *original_offset = 0;

  /*
     #ifdef LINUX_COMPIL
     struct timeval tv;
     unsigned int t_debut;
     gettimeofday(&tv, NULL);
     t_debut=tv.tv_usec;
     #endif
   */
  //ADDED
  if ((arbre_d =
          (struct arbre_data *) malloc (LOL_HARD_MALLOC *
              sizeof (struct arbre_data))) == NULL)
    return 0;

  if ((arbre =
          (struct arbre *) malloc (LOL_HARD_MALLOC * sizeof (struct arbre))) ==
      NULL)
    return 0;


  /* création ou lecture de la table des fréquences */
  //MODIFIED
  index_frequence =
      huffman_calculer_frequences (src, &nbre_octets, &nbre_ascii, size);

  dst = malloc (size);
  original_offset = dst;
  /*Ecriture de la taille en octets du fichier original */
  //REPLACED
  memcpy (dst, &nbre_octets, 4);
  dst += 4;
  //fwrite((char *)&nbre_octets, 4, 1, dst);
  /*Ecriture de la taille de la table des fréquences */
  //REPLACED
  memcpy (dst, &nbre_ascii, 2);
  dst += 2;
  //fwrite(&nbre_ascii, 2, 1, dst);

  /*Ecriture de la table des fréquences */
  i = index_frequence;
  while (i != -1) {
    nbre_ascii = i;
    //REPLACED
    memcpy (dst, &nbre_ascii, 1);
    dst++;
    memcpy (dst, &nbre_ascii, 4);
    dst += 4;
    //fwrite(&nbre_ascii, 1, 1, dst);
    //fwrite((char *)&arbre_d[i].frequence, 4, 1, dst);
    i = arbre_d[i].index_suivant;
  }

  /* Coinstruction de l'arbre à partir de la table des fréquences */
  racine_arbre = huffman_creer_arbre (index_frequence);

  /* Allocation de mémoire pour le dictionnaire */
  if ((dico =
          (struct dictionnaire *) malloc (256 *
              sizeof (struct dictionnaire))) == NULL) {
    /*free(arbre_d);
     *       free(arbre);*/
    perror ("malloc");
    return 0;
  }

  /* RAZ du champs taille du dico. Si on utilise une table de fréquences
   *    prédéfinie pour la compression, et qu'un caractère à compresser n'est pas
   *       présent dans la table, alors il ne sera pas traité */
  for (i = 0; i < 256; i++)
    dico[i].taille = 0;

  /* Création du dictionnaire à partir de l'arbre */
  huffman_creer_dictionnaire (code, racine_arbre, 0);

  /* Compression du fichier source et écriture dans le fichier cible */
  //DELETED
  //fseek(src, 0, SEEK_SET);
  code[0] = 0;
  bit_w = 0x80;
  /* Lecture de BUF_LEN octets dans le fichier source */
  //REPLACED
  /* Traitement octet par octet */
  for (i = 0; i < size; i++) {
    /* Ecriture du code correspondant au caractère dans le dictionnaire */
    octet_r = 0;
    bit_r = 0x80;
    /* Ecriture bit par bit */
    for (bit_count = 0; bit_count < dico[src[i]].taille; bit_count++) {
      if (dico[src[i]].code[octet_r] & bit_r)
        code[0] |= bit_w;
      /*else
       *                code[0]&=~(bit_w); */
      bit_r >>= 1;
      if (bit_r == 0) {
        octet_r++;
        bit_r = 0x80;
      }
      bit_w >>= 1;
      if (bit_w == 0) {
        /*printf("%3x", code[0]); */
        memcpy (dst, code, 1);
        dst++;
        //REPLACED
        //fputc(code[0], dst);
        code[0] = 0;
        bit_w = 0x80;
      }
    }
  }
  if (bit_w != 0x80) {
    memcpy (dst, code, 1);
    dst++;
    //REPLACED
    //fputc(code[0], dst);
  }

  free (dico);
  free (arbre_d);
  free (arbre);
  /* WHO CARES ? NOT ME
     #ifdef LINUX_COMPIL
     gettimeofday(&tv, NULL);
     printf("Compactage effectué en %u \xb5s. Taux de compression: %.2f\n", tv.tv_usec-t_debut, (float)nbre_octets/ftell(dst));
     #else
     printf("Compactage terminé. Taux de compression : %.2f\n", (float)nbre_octets/ftell(dst));
     #endif
   */
  //ADDED
  *output_size = (unsigned char *) dst - (unsigned char *) original_offset;
  return original_offset;
}

/*
 * Decompresse le fichier src
 * Le résultat se trouve dans dst
 * */
void *
huffman_decompacter (unsigned char *src, unsigned int *size)
{
  int i, j;
  unsigned long nbre_octets;
  unsigned char bit_r;
  short index_frequence;
  short racine_arbre;
  //unsigned char buffer[BUF_LEN];
  //ADDED
  unsigned char *dst = 0;
  unsigned char *original_offset = 0;

  //ADDED
  if ((arbre_d =
          (struct arbre_data *) malloc (LOL_HARD_MALLOC *
              sizeof (struct arbre_data))) == NULL)
    return 0;

  if ((arbre =
          (struct arbre *) malloc (LOL_HARD_MALLOC * sizeof (struct arbre))) ==
      NULL)
    return 0;
  /* Lecture de la taille du fichier original */
  //REPLACED
  memcpy (&nbre_octets, src, 4);
  dst += 4;
  *size = nbre_octets;
  dst = malloc (nbre_octets);
  original_offset = dst;
  //fread((char *)&nbre_octets, 4, 1, src);
  printf ("Décompression en cours...\n");

  /* Lecture de la table des fréquences */
  index_frequence = huffman_lire_frequences (src);

  if (index_frequence == -1) {
    printf ("Erreur de lecture de la table des frequences\n");
    return 0;
  }

  /* Construction de l'arbre à partir de la table des fréquences */
  racine_arbre = huffman_creer_arbre (index_frequence);

  /* Decompression du fichier source et écriture du résultat dans le fichier cible */
  j = racine_arbre;
  /* Lecture de BUF_LEN octets dans le fichier source */
  //REPLACED
  /* Traitement octet par octet */
  for (i = 0; nbre_octets > 0; i++) {
    /* Traitement bit par bit */
    for (bit_r = 0x80; bit_r != 0 && nbre_octets > 0; bit_r >>= 1) {
      if (src[i] & bit_r)
        j = arbre[j].branche1;
      else
        j = arbre[j].branche0;
      if ((arbre[j].branche0 == -1) || (arbre[j].branche1 == -1)) {
        /*printf("%c", j); */
        //REPLACED
        //fputc((char)j, dst);
        memcpy (dst, &j, 1);
        dst++;
        nbre_octets--;
        j = racine_arbre;
      }
    }
  }
  /*YOU HERE AGAIN ?
     #ifdef LINUX_COMPIL
     gettimeofday(&tv, NULL);
     printf("Décompression effectuée en %u\xb5s\n", tv.tv_usec-t_debut);
     #else
     printf("Décompression terminée\n");
     #endif
   */
  //ADDED
  free (arbre_d);
  free (arbre);
  return original_offset;
}

#ifdef ENABLE_MAIN
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


int
main (int argc, char **argv)
{
  char *file_path = "img.rgb";
  int file_fd = open (file_path, O_RDONLY);
  struct stat file_stat;
  stat (file_path, &file_stat);
  void *mapped_file =
      mmap (0, file_stat.st_size, PROT_READ, MAP_SHARED, file_fd, 0);
  void *compressed_buff = 0;
  void *uncompressed_buff = 0;
  unsigned int output_size = 0;


  compressed_buff = huffman_compacter ((unsigned char *) mapped_file,
      file_stat.st_size);
  uncompressed_buff = huffman_decompacter ((unsigned char *) compressed_buff,
      &output_size);

  return 0;
}

#endif
