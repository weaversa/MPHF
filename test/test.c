#include <sys/time.h>
#include <time.h>

#include "mphf.h"

int main() {

  struct timeval tv1;
  struct timezone tzp1;
  gettimeofday(&tv1, &tzp1);
  uint32_t random_seed = ((tv1.tv_sec & 0177) * 1000000) + tv1.tv_usec;

  fprintf(stderr, "random seed = %d\n", random_seed);
  srand(random_seed);

  uint32_t nElements = 30;
  uint32_t i, j;

  double fBitsPerElement = MPHFCalculateBound(nElements);

  fprintf(stdout, "Suggested bits-per-element for %u elements is %lf\n", nElements, fBitsPerElement);
  
  MPHFParameters test_parameters =
    { .fBitsPerElement = fBitsPerElement,
      .solver_string = "glucose-syrup -nthreads=16 -model"
    };
  
  MPHFBuilder *mphfb = MPHFBuilderAlloc(nElements);

  size_t nElementBytes = 10;
  uint8_t *pElement = malloc(nElementBytes * sizeof(uint8_t));
  if(pElement == NULL) {
    fprintf(stderr, "malloc() failed...exiting\n");
    return -1;
  }

  time_t start_wall = time(NULL);
  clock_t start_cpu = clock();

  struct timespec start_process, end_process;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_process);
  
  for(i = 0; i < nElements; i++) {
    for(j = 0; j < nElementBytes; j++) {
      pElement[j] = (uint8_t)(rand()%256);
    }
    if(MPHFBuilderAddElement(mphfb, pElement, nElementBytes) != 0) {
      fprintf(stderr, "Element insertion failed...exiting\n");
      return -1;
    }
  }
  
  MPHFQuerier *mphfq = MPHFBuilderFinalize(mphfb, test_parameters);

  if(mphfq == NULL) {
    fprintf(stderr, "MPHF Building Failed\n");
    return -1;
  }

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_process);
  clock_t end_cpu = clock();

  time_t end_wall = time(NULL);
  double time_wall = difftime(end_wall, start_wall);
  double time_cpu = ((double) (end_cpu - start_cpu)) / (double) CLOCKS_PER_SEC;

  uint64_t nanoseconds_process = ((double)(((end_process.tv_sec - start_process.tv_sec) * 1e9) + (end_process.tv_nsec - start_process.tv_nsec)));    
  
  fprintf(stdout, "\nBuilding took %1.0lf wallclock seconds and %1.0lf CPU seconds\n", time_wall, time_cpu);
  fprintf(stdout, "Building took %lu nanoseconds which is %lu nanoseconds per element\n", nanoseconds_process, nanoseconds_process / nElements);
  
  MPHFBuilderFree(mphfb);
  
  FILE *fout = fopen("mphf.out", "w");
  if(MPHFSerialize(fout, mphfq) != 0) {
    fprintf(stderr, "Serialization failed...exiting\n");
    return -1;
  }
  fclose(fout);
  MPHFQuerierFree(mphfq);

  fout = fopen("mphf.out", "r");
  mphfq = MPHFDeserialize(fout);
  if(mphfq == NULL) {
    fprintf(stderr, "Deserialization failed...exiting\n");
    return -1;
  }
  fclose(fout);

  uint8_t *seen = calloc(nElements, sizeof(uint32_t));
  
  srand(random_seed);
  for(i = 0; i < nElements; i++) {
    for(j = 0; j < nElementBytes; j++) {
      pElement[j] = (uint8_t)(rand()%256);
    }

    uint32_t key = MPHFQuery(mphfq, pElement, nElementBytes);
    seen[key] = 1;
  }

  free(pElement);
  
  uint32_t failures = 0;
  for(i = 0; i < nElements; i++) {
    if(seen[i] == 0) {
      failures++;
      fprintf(stderr, "Failure at %u\n", i);
    }
  }

  fprintf(stderr, "Passed = %4.2f%%\n", 100.0 * ((double) (nElements - failures)) / (double) nElements);

  free(seen);
  
  uint32_t queries_per_second = MPHFQueryRate(mphfq);
  fprintf(stdout, "\nTesting query speed with util func: %u queries per second (%u nanoseconds per query)\n", queries_per_second, 1000000000 / queries_per_second);

  uint32_t nMPHFBits = MPHFSize(mphfq);

  fprintf(stdout, "MPHF is %u total bits, %4.3lf bits per element\n", mphfq->nNumVariables, ((double) mphfq->nNumVariables) / (double) nElements);
  
  fprintf(stdout, "Seralized object uses %u total bits, %4.3lf bits per element\n", nMPHFBits, ((double) nMPHFBits) / (double) nElements);
  
  MPHFQuerierFree(mphfq);

  return 0;
}
