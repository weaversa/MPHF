#ifndef MPHFSERIAL_H
#define MPHFSERIAL_H

typedef struct MPHFSerialData {
  //MPHF Data
  uint8_t nNumElements;
  uint8_t nNumVariables;
} MPHFSerialData;

uint8_t MPHFSerialize(FILE *pMPHFFile, MPHFQuerier *mphfq);
MPHFQuerier *MPHFDeserialize(FILE *pMPHFFile);

#endif
