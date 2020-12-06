#include "mphf.h"

MPHFQuerier *MPHFQuerierAlloc(uint8_t nNumElements, uint8_t nNumVariables, uint8_t nNumUNSATCalls) {
  MPHFQuerier *mphfq = (MPHFQuerier *)malloc(1 * sizeof(MPHFQuerier));
  uint32_t nNumBlocks = ((nNumVariables-1) / 8) + 1;
  mphfq->pSolution = (uint8_t *)calloc(nNumBlocks, sizeof(uint8_t));
  mphfq->nNumElements = nNumElements;
  mphfq->nNumVariables = nNumVariables;
  mphfq->nNumUNSATCalls = nNumUNSATCalls;
  mphfq->bMMAP = 0;
  
  return mphfq;
}

void MPHFQuerierFree(MPHFQuerier *mphfq) {
  if(mphfq != NULL) {
    if(mphfq->bMMAP) {
      if(mphfq->pSolution != NULL) {
        uint8_t nNumBlocks = ((mphfq->nNumVariables-1) / 8) + 1;
        munmap(mphfq->pSolution, nNumBlocks * sizeof(uint8_t));
      }
      mphfq->bMMAP = 0;
    } else if(mphfq->pSolution != NULL) {
      free(mphfq->pSolution);
      mphfq->pSolution = NULL;
    }
    free(mphfq);
  }
}

MPHFQuerier *MPHFCreateQuerierFromBuilder(MPHFBuilder *mphfb, uint8_t *pSolution, uint8_t nNumVariables, uint8_t nNumUNSATCalls) {
  uint8_t i, j;

  MPHFQuerier *mphfq = MPHFQuerierAlloc(mphfb->pHashes.nLength, nNumVariables, nNumUNSATCalls);
  uint32_t nNumBlocks = ((nNumVariables-1) / 8) + 1;

  for(i = 0; i < nNumBlocks; i++) {
    for(j = 0; j < 8; j++) {
      if(1 + i*8 + j > mphfq->nNumVariables) break;
      if(pSolution[1 + i*8 + j] == 1) {
        mphfq->pSolution[i] |= 1 << j;
      }
    }
  }

  free(pSolution);
  
  return mphfq;
}

//Can be used to query externally hashed elements
uint32_t MPHFQueryHash(MPHFQuerier *mphfq, MPHFHash mphfh) {
  uint32_t i;

  uint32_t nLitsPerClause = 3;
  while (mphfq->nNumElements > (1<<nLitsPerClause)) nLitsPerClause++;

  int8_t pList[nLitsPerClause];
  clause_t vector;
  vector.pList = pList;
  vector.nLength_max = nLitsPerClause;
  
  MPHFGenerateVectorFromHash(mphfh, &vector, mphfq->nNumVariables, mphfq->nNumUNSATCalls);

  //Evaluate clause
  uint32_t nKey = 0;
  for(i = 0; i < nLitsPerClause; i++) {
    int8_t nLit = pList[i];
    uint8_t bit;
    if(nLit > 0) {
      bit = (mphfq->pSolution[(nLit-1)>>3] >> ((nLit-1)&0x7)) & 1;
    } else {
      bit = (~(mphfq->pSolution[((-nLit)-1)>>3] >> (((-nLit)-1)&0x7))) & 1;
    }
    nKey |= bit << i;
  }

  if(nKey >= mphfq->nNumElements) nKey ^= 1 << (nLitsPerClause-1);
  
  return nKey;
}

uint32_t MPHFQuery(MPHFQuerier *mphfq, const void *pElement, size_t nElementBytes) {
  MPHFHash mphfh = MPHFGenerateHashesFromElement(pElement, nElementBytes);
  return MPHFQueryHash(mphfq, mphfh);
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

uint64_t MPHFSize(MPHFQuerier *mphfq) {
  uint8_t nNumBlocks = ((mphfq->nNumVariables-1) / 8) + 1;
  uint32_t nMPHFBits = 8 + 8 + (8 * nNumBlocks);
  return nMPHFBits;
}
