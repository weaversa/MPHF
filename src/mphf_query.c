#include "mphf.h"

MPHFQuerier *MPHFQuerierAlloc(uint8_t nNumElements, uint8_t nNumVariables) {
  MPHFQuerier *mphfq = (MPHFQuerier *)malloc(1 * sizeof(MPHFQuerier));
  uint32_t nNumBlocks = (nNumVariables / 8) + 1;
  mphfq->pSolution = (uint8_t *)calloc(nNumBlocks, sizeof(uint8_t));
  mphfq->nNumElements = nNumElements;
  mphfq->nNumVariables = nNumVariables;
  
  return mphfq;
}

void MPHFQuerierFree(MPHFQuerier *mphfq) {
  if(mphfq != NULL) {
    if(mphfq->pSolution != NULL) {
      free(mphfq->pSolution);
      mphfq->pSolution = NULL;
    }
    free(mphfq);
  }
}

MPHFQuerier *MPHFCreateQuerierFromBuilder(MPHFBuilder *mphfb, uint8_t *pSolution, uint8_t nNumVariables) {
  uint8_t i, j;

  MPHFQuerier *mphfq = MPHFQuerierAlloc(mphfb->pHashes.nLength, nNumVariables);
  uint32_t nNumBlocks = (nNumVariables / 8) + 1;
  fprintf(stderr, "num blocks = %u (%u)\n", nNumBlocks, nNumVariables);

  //We're currently storing one extra bit here. pSolution[0] currently
  //references variable 0, which there is none.
  for(i = 0; i < nNumBlocks; i++) {
    for(j = 0; j < 8; j++) {
      if(i*8 + j > mphfq->nNumVariables) break;
      if(pSolution[i*8 + j] == 1) {
        mphfq->pSolution[i] |= 1 << j;
      }
    }
  }

  return mphfq;
}

uint32_t MPHFQuery(MPHFQuerier *mphfq, const void *pElement, size_t nElementBytes) {
  uint32_t i;

  MPHFHash mphfh = MPHFGenerateHashesFromElement(pElement, nElementBytes);
  uint32_t nLitsPerClause = 3;
  while (mphfq->nNumElements > (1<<nLitsPerClause)) nLitsPerClause++;

  int8_t pList[nLitsPerClause];
  clause_t vector;
  vector.pList = pList;
  vector.nLength_max = nLitsPerClause;
  
  MPHFGenerateVectorFromHash(mphfh, &vector, mphfq->nNumVariables);

  //Evaluate clause
  uint32_t nKey = 0;
  for(i = 0; i < nLitsPerClause; i++) {
    int8_t nLit = pList[i];
    uint8_t bit;
    if(nLit > 0) {
      bit = (mphfq->pSolution[nLit>>3] >> (nLit&0x7)) & 1;
    } else {
      bit = (~(mphfq->pSolution[(-nLit)>>3] >> ((-nLit)&0x7))) & 1;
    }
    nKey |= bit << i;
  }

  if(nKey >= mphfq->nNumElements) nKey ^= 1 << (nLitsPerClause-1);
  
  return nKey;
}

uint32_t MPHFQueryRate(MPHFQuerier *mphfq) {
  uint32_t i;
  uint32_t nElementsQueried = 10000000;

  clock_t start = clock();
  for(i = 0; i < nElementsQueried; i++) {
    uint32_t volatile ret = MPHFQuery(mphfq, &i, sizeof(uint32_t));
  }
  clock_t end = clock();
  
  double time_elapsed_in_seconds = ((double) (end - start)) / (double) CLOCKS_PER_SEC;
  return (uint32_t) (((double) nElementsQueried) / time_elapsed_in_seconds);
}
