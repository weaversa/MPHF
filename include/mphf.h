#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>

#include "../lib/c_list_types/include/c_list_types.h"

#include "list_types.h"
#include "mphf_hashes.h"

#define MPHF_PRINT_BUILD_PROCESS

typedef struct MPHFParameters {
  double fBitsPerElement; //Roughly 1.44
  char solver_string[256]; //"glucose -model", for example
} MPHFParameters;

typedef struct MPHFBuilder {
  MPHFHash_list pHashes;
} MPHFBuilder;

typedef struct MPHFQuerier {
  uint8_t *pSolution;
  uint8_t nNumElements;
  uint8_t nNumVariables;
  uint8_t bMMAP;
} MPHFQuerier;

#include "mphf_serial.h"

MPHFBuilder *MPHFBuilderAlloc(uint32_t nExpectedElements);
void MPHFBuilderFree(MPHFBuilder *mphfb);
uint8_t MPHFBuilderAddElement(MPHFBuilder *mphfb, const void *pElement, size_t nElementBytes);
MPHFQuerier *MPHFBuilderFinalize(MPHFBuilder *mphfb, MPHFParameters params);
MPHFQuerier *MPHFCreateQuerierFromBuilder(MPHFBuilder *mphfb, uint8_t *pSolution, uint8_t nNumVariables);
uint32_t MPHFQuery(MPHFQuerier *mphfq, const void *pElement, size_t nElementBytes);
void MPHFQuerierFree(MPHFQuerier *mphfq);
uint32_t MPHFQueryRate(MPHFQuerier *mphfq);
uint8_t *find_solution_external(cnf_t *pCNF, uint32_t nNumVariables, char solver_string[256]);
