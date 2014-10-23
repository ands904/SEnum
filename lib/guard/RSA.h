// RSA public key encryption

#include "vlong.h"

class public_key
{
  public:
  vlong m,e,d;
  vlong encrypt( const vlong& plain ); // Requires 0 <= plain < m
};
