#!/usr/bin/env cpx
#!
  INFO(VARVALS( sizeof(long),sizeof(long long),sizeof(__int128_t)));

  bitset<177> bits;
  bits[0]=1;
  while(bits.any())
  {

    INFO(VARVAL(bits));
    bits<<=1;
  }

