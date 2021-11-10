#ifndef __CRYPTO_H__
#define __CRYPTO_H__


#include <am.h>
#include <klib.h>
#include <klib-macros.h>

void aes64ds01_entry();

void sha256sig0_entry();
void sha256sig1_entry();
void sha256sum0_entry();
void sha256sum1_entry();
void sha512sig0_entry();
void sha512sig1_entry();
void sha512sum0_entry();
void sha512sum1_entry();

#endif
