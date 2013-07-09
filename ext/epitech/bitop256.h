#ifndef		__BITOP256_H
#define		__BITOP256_H
 void ClearAll256 (unsigned char *bits);
int TestBit256 (const unsigned char *bits, unsigned char bit);
void Copy256 (unsigned char *dest, const unsigned char *src);
void LeftShift256 (unsigned char *bits, int shifts);
void RightShift256 (unsigned char *bits, int shifts);
 
#endif /*  */
