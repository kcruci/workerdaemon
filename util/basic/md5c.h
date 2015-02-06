/*
	 MD5.H - header file for MD5C.C
	RSA Data Security, Inc. MD5 Message-Digest Algorithm
*/

#ifndef _MD5C_H
#define _MD5C_H

/* POINTER_MD5 defines a generic pointer type */
//typedef unsigned char * POINTER_MD5;

/* UINT2 defines a two byte word */
//typedef unsigned short int UINT2;

/* unsigned int defines a four byte word */
//typedef unsigned int unsigned int;

/* MD5 context. */
typedef struct {
  unsigned int state[4];                                           /* state (ABCD) */
  unsigned int count[2];                /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                                 /* input buffer */
} MD5_CTX;

	
void MD5cInit(MD5_CTX *);
void MD5cUpdate(MD5_CTX *, unsigned char *, unsigned int);
void MD5cFinal(unsigned char [16], MD5_CTX *);


#endif
