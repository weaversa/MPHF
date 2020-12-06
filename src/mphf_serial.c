#include "mphf.h"

uint8_t MPHFSerialize(FILE *pMPHFFile, MPHFQuerier *mphfq) {
  if(pMPHFFile == NULL) return 1; //Failure
  
  size_t write;

  uint8_t nNumBlocks = ((mphfq->nNumVariables-1) / 8) + 1;
  
  //Write mphf solution bytes
  write = fwrite(mphfq->pSolution, sizeof(uint8_t), nNumBlocks, pMPHFFile);
  if(write != nNumBlocks) return 1; //Failure

  //Write mphf header
  MPHFSerialData mphfsd = { .nNumElements   = mphfq->nNumElements,
                            .nNumVariables  = mphfq->nNumVariables,
                            .nNumUNSATCalls = mphfq->nNumUNSATCalls
                          };
    
  write = fwrite(&mphfsd, sizeof(MPHFSerialData), 1, pMPHFFile);
  if(write != 1) return 1; //Failure

  return 0; //Success
}

MPHFQuerier *MPHFDeserialize(FILE *pMPHFFile) {
  if(pMPHFFile == NULL) return NULL;
  
  int seek = fseek(pMPHFFile, -(int32_t) sizeof(MPHFSerialData), SEEK_END);
  if(seek != 0) return NULL;

  //Record the file size (minus the header data) for later verification
  size_t read;

  //Read filter header
  MPHFSerialData mphfsd;
  read = fread(&mphfsd, sizeof(MPHFSerialData), 1, pMPHFFile);
  if(read != 1) return NULL;

  MPHFQuerier *mphfq = (MPHFQuerier *)malloc(1 * sizeof(MPHFQuerier));
  if(mphfq == NULL) return NULL;
  mphfq->nNumElements = mphfsd.nNumElements;
  mphfq->nNumVariables = mphfsd.nNumVariables;
  mphfq->nNumUNSATCalls = mphfsd.nNumUNSATCalls;

  uint8_t nNumBlocks = ((mphfq->nNumVariables-1) / 8) + 1;
  mphfq->pSolution = (uint8_t *)mmap(0, nNumBlocks * sizeof(uint8_t), PROT_READ, MAP_PRIVATE, fileno(pMPHFFile), 0);

  mphfq->bMMAP = 1;

  return mphfq;
}
