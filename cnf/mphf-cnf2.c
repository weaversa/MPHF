#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <time.h>

#include "sha512.h"

int removeDups (int *clause, int s) {
  int i, j;
  for (i = 0; i < s-1; i++)
    for (j = i+1; j < s; j++)
      if (clause[i] == clause[j])   //Duplicate literal
        clause[j--] = clause[--s];

  return s;
}

int isTautology (int *clause, int s) {
  int i, j;
  for (i = 0; i < s-1; i++)
    for (j = i+1; j < s; j++)
      if (clause[i] == -clause[j])  //Tautology
        return 1;

  return 0;
}

int compare (int *x, int *y, int k, int n, int c) {
  int i, j, l = 0, v = 0;
  int count = 0;
  if (n != (1<<k)) { v = 1<<(k-1); l = n - v; }

  int clause[2*k];

  for (i = 0; i < n; i++) {
    int b = k; if ((i >= l) && (i < v)) b--;

    for (j = 0; j < b; j++) {
      int pos = (i>>j) & 1;
      if (pos) {
        clause[j*2  ] = -x[j];
        clause[j*2+1] = -y[j]; }
      else {
        clause[j*2  ] =  x[j];
        clause[j*2+1] =  y[j]; }
    }

    if (!isTautology (clause, 2*b)) {
      if (c) { count++; }
      else {
        int size = removeDups (clause, 2*b);
        for (j = 0; j < size; j++) {
          printf ("%i ", clause[j]); }
        printf ("0\n"); }
    }
  }
  return count;
}

int main (int argc, char** argv) {
  int h, i, j, n, m;
  int k = 3;

  struct timeval tv1;
  struct timezone tzp1;
  gettimeofday (&tv1, &tzp1);
  int seed = ((tv1.tv_sec & 0177) * 1000000) + tv1.tv_usec;

  if (argc > 1) {
    n = atoi (argv[1]); while (n > (1<<k)) k++; }
  else {
    printf ("c use as follows: ./mphf-cnf #ELEMENTS #VARIABLES [SEED]\n"); exit (0); }

  if (argc > 2) {
    m = atoi (argv[2]); }
  else {
    printf ("c use as follows: ./mphf-cnf #ELEMENTS #VARIABLES [SEED]\n"); exit (0); }

  if (argc > 3) {
    seed = atoi (argv[3]); }

  fprintf (stderr, "c random seed = %d\n", seed);
  srand (seed);

  size_t nElementBytes = 10;
  uint8_t *pElement = malloc(nElementBytes * sizeof(uint8_t));

  int **hash;
  hash = (int**) malloc (sizeof (int*) * n);
  for (i = 0; i < n; i++) {

    for(j = 0; j < nElementBytes; j++)
      pElement[j] = (uint8_t)(rand() % 256);

    hash[i] = (int*) malloc (sizeof(int) * k);

    uint16_t shahash[32];
    SHA512_Simple (pElement, (int)nElementBytes, (unsigned char *)shahash);

    uint32_t shaindex = 0;
    for (j = 0; j < k; j++) {
      uint32_t max  = 0xffff;

      while (shahash[shaindex] >= (max - (max % (2*m)))) { shaindex++; }
      int htry = (shahash[shaindex++] % (2*m));
      hash[i][j] = (htry & 1) ? (-htry-1)/2 : 1 + (htry/2);

      //Remove literal if it or its complement already exists
      for (h = 0; h < j; h++)
        if ((abs(hash[i][h])) == abs(hash[i][j])) { j--; break; }
    }
  }

  int nCls = 0, *used;
  used = (int *) malloc (sizeof (int) * (m+1));
  for (i = 1; i <= m; i++) used[i] = 0;
  for (i = 0; i <  n; i++)
    for (j = 0; j < k; j++)
       used[abs(hash[i][j])] = 1;

  for (i = 0; i <  n; i++) {
    printf ("c hash[%i]: ", i);
    for (j = 0; j < k; j++)
       printf ("%i ", hash[i][j]);
    printf ("\n"); }


  for (i = 1; i <= m; i++)
    if (!used[i])
      nCls++;

  for (h = 0; h < n; h++)
    for (i = h + 1; i < n; i++)
      nCls += compare (hash[h], hash[i], k, n, 1);

  printf ("p cnf %i %i\n", m, nCls);

  for (i = 1; i <= m; i++)
    if (!used[i])
      printf ("-%i 0\n", i);

  for (h = 0; h < n; h++)
    for (i = h + 1; i < n; i++)
      compare (hash[h], hash[i], k, n, 0);
}
