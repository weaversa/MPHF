#ifndef MPHF_HASHES
#define MPHF_HASHES

#include "xxhash.h"

typedef struct MPHFHash {
  uint64_t h1;
} MPHFHash;

create_c_list_headers(MPHFHash_list, MPHFHash)

MPHFHash MPHFGenerateHashesFromElement(const void *pElement, size_t nElementBytes);
void MPHFGenerateVectorFromHash(MPHFHash mphfh, clause_t *pVector, uint32_t nSize);

#endif
