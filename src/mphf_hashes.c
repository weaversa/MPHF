#include "mphf.h"

create_c_list_type(MPHFHash_list, MPHFHash)

MPHFHash MPHFGenerateHashesFromElement(const void *pElement, size_t nElementBytes) {
  MPHFHash mphfh;
  uint64_t nonce = 0;
  assert(nElementBytes < 0x7fffffff);

  do {
    mphfh.h1 = XXH64(pElement, nElementBytes, (unsigned long long)0xa7cb5c58462b6c85 + (nonce++));
  } while (mphfh.h1 == 0);
  
  return mphfh;
}

//Here we use the Kirsch-Mitzenmacher technique.
inline
int8_t MPHFGenerateIthValueFromHash(MPHFHash mphfh, uint32_t nSize, uint32_t i, uint32_t nonce) {
  uint32_t *mphfh_32 = (uint32_t *)&mphfh;
  uint32_t hash = ((mphfh_32[0] + (mphfh_32[1] * (i + nonce))) >> i) % (2*nSize);
  //The '>> i' ensures a new parity bit for each i
  return hash & 1 ? (int8_t) (-hash-1)>>1 : (int8_t) 1 + (hash>>1);
}

void MPHFGenerateVectorFromHash(MPHFHash mphfh, clause_t *pVector, uint32_t nSize, uint32_t nonce) {
  uint32_t i;

  pVector->nLength = pVector->nLength_max;
  
  for(i = 0; i < pVector->nLength; i++) {
    pVector->pList[i] = MPHFGenerateIthValueFromHash(mphfh, nSize, i, nonce);
  }
}
