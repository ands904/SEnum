#ifndef _IDEA_DEFINED

#define _IDEA_DEFINED

/* Defines for the PGP-style types used in IDEA.C */

typedef unsigned char byte;	
typedef unsigned short int word16;
typedef unsigned long int word32;	

/* Macros from PGP */

#define burn(x)	memset( x, 0, sizeof( x ) )

/* IDEA algorithm constants */

#define IDEAKEYSIZE		16
#define IDEABLOCKSIZE	8

#define IDEAROUNDS		8
#define IDEAKEYLEN		( 6 * IDEAROUNDS + 4 )

/* Routines used to implement the IDEA encryption */
void ideaExpandKey( byte const *userkey, word16 *EK );
void ideaInvertKey( word16 const *EK, word16 DK[IDEAKEYLEN] );
void ideaCipher( byte const (inbuf[8]), byte (outbuf[8]), word16 const *key );

#endif /* _IDEA_DEFINED */
