/**
 * Small test program to validate that the small implementation by Matt Mahoney
 * produces valid SHA1 hashes.
 * author: Christophe VG
 */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Wpointer-sign"
#pragma clang diagnostic ignored "-Wincompatible-pointer-types"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>

#include "sha1.h"

void sha1_dump(unsigned char buffer[20]) {
  int i;
  for (i = 0; i < 20; i++) {
      printf("%02x ", buffer[i]);
  }
  printf("\n");
}

int main() {
  unsigned char ibuf[] = "hello world";
  unsigned char obuf[20];
  unsigned char obuf2[20];

  // OpenSSL implementation
  SHA1(ibuf, strlen(ibuf), obuf);

  // Matt Mahoney implementation
  SHA1Context context;

  SHA1Reset(&context);
  SHA1Input(&context, &ibuf, strlen(ibuf));
  SHA1Result(&context, &obuf2);

  // visual confirmation ;-)
  sha1_dump(obuf);
  sha1_dump(obuf2);

  // make sure ;-)
  for(int i=0; i<20; i++) {
    assert( obuf[i] == obuf2[i] );
  }

  printf("RESULT: OK\n");

  return 0;
}

#pragma clang diagnostic pop
