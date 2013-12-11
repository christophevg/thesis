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

  sha1_t openssl,
         mahoney,
         allinone;

  // OpenSSL implementation
  SHA1(ibuf, strlen(ibuf), &(openssl.hash));

  // Matt Mahoney implementation
  SHA1Context context;

  SHA1Reset (&context);
  SHA1Input (&context, &ibuf, strlen(ibuf));
  SHA1Result(&context, &(mahoney.hash));

  // visual confirmation ;-)
  // sha1_dump(openssl.hash);
  // sha1_dump(mahoney.hash);

  // make sure ;-)
  assert( memcmp( openssl.hash, mahoney.hash, 20) == 0 );

  // use the all in one compute function
  allinone = SHA1Compute(&ibuf, strlen(ibuf));

  // more visual confirmation ;-)
  // sha1_dump(allinone.hash);

  assert( allinone.result == shaSuccess );
  assert( memcmp(openssl.hash, allinone.hash, 20) == 0 );

  printf("RESULT: OK\n");

  return 0;
}

#pragma clang diagnostic pop
