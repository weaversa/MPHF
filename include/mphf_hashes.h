#ifndef MPHF_HASHES
#define MPHF_HASHES

#define XXH_INLINE_ALL
#define XXH_STATIC_LINKING_ONLY   /* access advanced declarations */
#define XXH_IMPLEMENTATION   /* access definitions */
//#include "../lib/xxHash/xxhash.h"                                                                                                                                                           
#include "../lib/xxHash/xxh3.h"

typedef struct MPHFHash {
  uint64_t h1;
} MPHFHash;

create_c_list_headers(MPHFHash_list, MPHFHash)

MPHFHash MPHFGenerateHashesFromElement(const void *pElement, size_t nElementBytes);
void MPHFGenerateVectorFromHash(MPHFHash mphfh, clause_t *pVector, uint32_t nSize);

#endif
