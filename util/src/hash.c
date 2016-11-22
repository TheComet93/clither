#include "util/hash.h"
#include "util/string.h"
#include <string.h>

/*
 * Shift operations in C are only defined for shift values which are
 * not negative and smaller than sizeof(value) * CHAR_BIT.
 * The mask, used with bitwise-and (&), prevents undefined behaviour
 * when the shift count is 0 or >= the width of unsigned int.
 */

#include <stdint.h>  /* for uint32_t, to get 32bit-wide rotates, regardless of the size of int. */
#include <limits.h>  /* for CHAR_BIT */

uint32_t rotl32 (uint32_t value, unsigned int count) {
    const unsigned int mask = (CHAR_BIT*sizeof(value)-1);
    count &= mask;
    return (value<<count) | (value>>( (-count) & mask ));
}

uint32_t rotr32 (uint32_t value, unsigned int count) {
    const unsigned int mask = (CHAR_BIT*sizeof(value)-1);
    count &= mask;
    return (value>>count) | (value<<( (-count) & mask ));
}

/* ------------------------------------------------------------------------- */
uint32_t
hash_jenkins_oaat(const char* message, uint32_t len)
{
    uint32_t hash, i;
    for(hash = i = 0; i != len; ++i)
    {
    	hash += message[i];
    	hash += (hash << 10);
    	hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 1);
    hash += (hash << 15);
    return hash;
}

/* ------------------------------------------------------------------------- */
const char* sha256_get_next_chunk(const char* message,
                                         uint32_t len,
                                         char current_chunk[16])
{
    uint32_t offset = *(uint32_t*)current_chunk;
    *(uint32_t*)current_chunk += 16;

    if(*(uint32_t*)current_chunk < len)
        return message + offset;

    {   int i = len - offset;
        memcpy(current_chunk, message + offset, i);
        /*current_chunk[i];*/
        for(++i; i != 12; ++i)
        {

        }
    }

    return NULL;
}

void hash_sha256(const char* message, uint32_t len, uint32_t digest[8])
{
    /*
     * Initialise array of round constants
     * (first 32 bits of the fractional parts of the cube roots of the first 64
     * primes 2..311)
     *
    static const uint32_t k[] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };*/

    char current_chunk[16];
    /*const char* chunk;*/

    /*
     * Initial hash values
     * (first 32 bits of the fractional parts of the square roots of the first
     * 8 primes 2..19)
     */
    digest[0] = 0x6a09e667;
    digest[1] = 0xbb67ae85;
    digest[2] = 0x3c6ef372;
    digest[3] = 0xa54ff53a;
    digest[4] = 0x510e527f;
    digest[5] = 0x9b05688c;
    digest[6] = 0x1f83d9ab;
    digest[7] = 0x5be0cd19;

    *(uint32_t*)current_chunk = 0;
/*
    for(chunk = message; chunk; chunk = sha256_get_next_chunk(message, len, current_chunk))
    {
        uint32_t i;
        uint32_t w[64];
        uint32_t a[8];

        memcpy(&w, message, len);
        memcpy(a, h, 8);

        for(i = 16; i != 64; ++i)
        {
            uint32_t s0 = rotr32(w[i-15], 7) ^ rotr32(w[i-15], 18) ^ (w[i-15] >> 3);
            uint32_t s1 = rotr32(w[i-2], 17) ^ rotr32(w[i-2], 19) ^(w[i-2] >> 10);
            w[i] = w[i-16] + s0 + w[i-7] + s1;
        }

        for(i = 0; i != 64; ++i)
        {
            uint32_t s1 = rotr32(a[4], 6) ^ rotr32(a[4], 11) ^ rotr32(a[4], 25);
            uint32_t ch = (a[4] & a[5]) ^ (~a[4] & a[6]);
            uint32_t temp1 = a[7] + s1 + ch + k[i] + w[i];
            uint32_t s0 = rotr32(a[0], 2) ^ rotr32(a[0], 13) ^ rotr32(a[0], 22);
            uint32_t maj = (a[0] & a[1]) ^ (a[0] & a[2]) ^ (a[1] & a[2]);
            uint32_t temp2 = s0 + maj;
        }

        digest[0] += temp1 + temp2;
        digest[1] += a[0];
        digest[2] += a[1];
        digest[3] += a[2];
        digest[4] += a[3] + temp1;
        digest[5] += a[4];
        digest[6] += a[5];
        digest[7] += a[6];
    }*/
}
