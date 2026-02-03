#include <am.h>
#include <klib.h>
#include <klib-macros.h>

// #include <stdint.h>
// #include <string.h>
// #include <stdio.h>

#ifndef N
#define N 56
#endif

#define sha256_hash_block sha256_block_lmul1_multi

void sha256_block_lmul1_single(
  uint32_t hash[8],
  const void *block
);

void sha256_block_lmul1_multi(
  uint32_t hash[8],
  const void *block,
  size_t block_num
);

void sha256_hash_init(uint32_t hash[8])
{
  hash[0] = 0x9b05688cUL; // f
  hash[1] = 0x510e527fUL; // e
  hash[2] = 0xbb67ae85UL; // b
  hash[3] = 0x6a09e667UL; // a
  hash[4] = 0x5be0cd19UL; // h
  hash[5] = 0x1f83d9abUL; // g
  hash[6] = 0xa54ff53aUL; // d
  hash[7] = 0x3c6ef372UL; // c
}

void print_sha256_hash(const uint32_t hash[8])
{
  printf("%08x ", hash[3]); // a
  printf("%08x ", hash[2]); // b
  printf("%08x ", hash[7]); // c
  printf("%08x ", hash[6]); // d
  printf("%08x ", hash[1]); // e
  printf("%08x ", hash[0]); // f
  printf("%08x ", hash[5]); // g
  printf("%08x ", hash[4]); // h
  printf("\n");
}

void sha256_hash(
  uint32_t H[8], //!< in,out - message block hash
  uint8_t *M,    //!< in - The message to be hashed
  size_t len     //!< Length of the message in *bytes*.
)
{
  uint32_t p_B[16];
  uint8_t *p_M = M;

  size_t len_bits = len << 3;

  size_t block_num = len >> 6;

  if (block_num) {
    sha256_hash_block(H, p_M, block_num);
    len -= 64 * block_num;
  }

  // use this instead if using sha256_block_lmul1_single
  // while (len >= 64)
  // {
  //   sha256_hash_block(H, p_M); // Digest another block
  //   p_M += 64; // Adjust pointers and length.
  //   len -= 64;
  // }

  memcpy(p_B, p_M, len); // Copy remaining bytes into block

  uint8_t *bp = (uint8_t *)p_B;
  bp[len++] = 0x80; // Append `1` to end of message

  if (len > 56)
  {                                // Do we spill into another block?
    memset(bp + len, 0, 64 - len); // If yes, clear rest of this block
    sha256_hash_block(H, p_B, 1);  // Length will be added in a new block
    len = 0;                       //
  }

  size_t i = 64;
  while (len_bits)
  { // Add length to end of this block
    bp[--i] = len_bits & 0xFF;
    len_bits = len_bits >> 8;
  }

  memset(bp + len, 0, i - len); // Clear fstart of block/EoM to len

  sha256_hash_block(H, p_B, 1);
}

#define INIT()            \
  do                      \
  {                       \
    asm volatile(         \
      "lui a0,0x2\n"      \
      "addiw a0,a0,512\n" \
      "csrs mstatus,a0\n" \
      "csrwi vcsr,0" ::); \
  } while (0)

uint8_t data[N];

int main()
{
  INIT();

  uint32_t hash[8];
  sha256_hash_init(hash);

  uint64_t scycle, ecycle;
  uint64_t sinst, einst;

  asm volatile("csrr %0, mcycle" : "=r"(scycle));
  asm volatile("csrr %0, minstret" : "=r"(sinst));
  sha256_hash(hash, data, N);

  asm volatile("csrr %0, mcycle" : "=r"(ecycle));
  asm volatile("csrr %0, minstret" : "=r"(einst));

  print_sha256_hash(hash);

  printf("SHA256 simple benchmark\n");
  printf("SHA256: chunk=%llu bytes, iterations=1, cycles=%llu, insts=%llu\n", N, ecycle - scycle, einst - sinst);

  return 0;
}