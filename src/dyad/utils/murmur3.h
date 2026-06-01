//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the
// public domain. The author hereby disclaims copyright to this source
// code.

#ifndef _MURMURHASH3_H_
#define _MURMURHASH3_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------

void MurmurHash3_x86_32 (const void *key, int len, uint32_t seed, void *out);

void MurmurHash3_x86_128 (const void *key, int len, uint32_t seed, void *out);

/**
 * @brief Computes a 128-bit MurmurHash3 hash for a 64-bit platform.
 *
 * @details
 * Implements the MurmurHash3_x64_128 variant, which produces a 128-bit hash
 * optimized for 64-bit platforms. The input is processed in 16-byte blocks
 * using two 64-bit hash accumulators (@c h1 and @c h2), with any remaining
 * bytes handled individually in the tail section. Both accumulators are
 * finalized via @c fmix64() to ensure good avalanche behavior, then written
 * to @p out as two consecutive @c uint64_t values.
 *
 * This is a well-known non-cryptographic hash function suitable for hash
 * tables and data fingerprinting. It is not suitable for cryptographic or
 * security-sensitive purposes.
 *
 * @param[in]  key   Pointer to the data to hash. Must not be @c NULL.
 * @param[in]  len   Length of @p key in bytes.
 * @param[in]  seed  Seed value to initialize both hash accumulators. Different
 *                   seeds produce different hash values for the same input.
 * @param[out] out   Pointer to a buffer of at least 16 bytes to receive the
 *                   128-bit hash result, stored as two consecutive @c uint64_t
 *                   values (@c h1 followed by @c h2).
 *
 * @note This implementation is optimized for 64-bit platforms. Results may
 *       differ from the 32-bit variant (@c MurmurHash3_x86_128) for the same
 *       input.
 * @note This is not a cryptographic hash function and must not be used for
 *       security-sensitive purposes such as password hashing or MACs.
 */
void MurmurHash3_x64_128 (const void *key, int len, uint32_t seed, void *out);

//-----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif  // _MURMURHASH3_H_
