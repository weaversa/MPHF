#include <stdio.h>
#include <stdlib.h>

#include "sha512.h"

int main (int argc, char** argv) {
  int h, i, j, n, m, seed;
  int k = 3;

  FILE* solution;

  if (argc > 1) {
    n = atoi (argv[1]); while (n > (1<<k)) k++; }
  else {
    printf ("c use as follows: ./decode #ELEMENTS #VARS SEED SOLUTION\n"); exit (0); }

  if (argc > 2) {
    m = atoi (argv[2]); }
  else {
    printf ("c use as follows: ./decode #ELEMENTS #VARS SEED SOLUTION\n"); exit (0); }

  if (argc > 3) {
    seed = atoi (argv[3]); }
  else {
    printf ("c use as follows: ./decode #ELEMENTS #VARS SEED SOLUTION\n"); exit (0); }
  srand (seed);

  if (argc > 4) {
    solution = fopen (argv[4], "r"); }
  else {
    solution = stdin;
    printf ("c expecting solution in stdin\n"); }


  int *sol;
  sol = (int*) malloc (sizeof (int) * m);
  i = 0;
  while (i < m) {
    char bit;
    int tmp = fscanf (solution, "%c", &bit);
    if (tmp == EOF) break;
    else sol[i++] = (int) (bit - '0');
  }
  if (i == 0) { printf ("c no solution file (formula UNSAT?)\n"); exit (0); }

  printf ("c m: %i\n", m);

  size_t nElementBytes = 10;
  uint8_t *pElement = malloc(nElementBytes * sizeof(uint8_t));

  int **hash;
  hash = (int**) malloc (sizeof (int*) * n);
  for (i = 0; i < n; i++) {
    for (j = 0; j < nElementBytes; j++)
      pElement[j] = (uint8_t)(rand()%256);

    hash[i] = (int*) malloc (sizeof(int) * k);

    uint16_t shahash[32];
    SHA512_Simple (pElement, (int)nElementBytes, (unsigned char *)shahash);

    uint32_t shaindex = 0;
    for (j = 0; j < k; j++) {
      uint32_t max  = 0xffff;

      while (shahash[shaindex] >= (max - (max % (2*m)))) { shaindex++; }
      int htry = (shahash[shaindex++] % (2*m));
      hash[i][j] = (htry & 1) ? (-htry-1)/2 : 1 + (htry/2);

      //Remove literal if it or its complements already exists
      for (h = 0; h < j; h++)
        if ((abs(hash[i][h])) == abs(hash[i][j])) { j--; break; }
    }
  }

  for (i = 0; i < n; i++) {
    printf ("c hash[");
    if (i < 10) printf (" ");
    printf("%i]: ", i);
    for (j = 0; j < k; j++)
      printf ("%i (%i) ", hash[i][j], sol[ abs(hash[i][j])-1 ]);
//      printf ("%i (%i) ", hash[i][j] + 1, sol[ hash[i][j] ]);
    printf ("\n"); }

  int *map;
  map = (int*) malloc (sizeof (int) * n);
  int *rev;
  rev = (int*) malloc (sizeof (int) * n);
  for (i = 0; i < n; i++) rev[i] = -1;

  int offset = 1 << (k-1);
  int flag = 1;
  for (i = 0; i < n; i++) {
    int r = 0;
    for (j = 0; j < k; j++) {
      if (sol[ abs(hash[i][j])-1 ] == (hash[i][j]>0)) r += 1 << j;
//      if (sol[ hash[i][j] ] == 1) r += 1 << j;
    }
    if (r >= n) r -= offset;
    map[i] = r;
    if ((r < 0) || (r >= n) || (rev[r] != -1)) {
      flag = 0;
      printf ("c ERROR\n");
    }
    rev[r] = i;
    printf ("c map[");
    if (i < 10) printf (" ");
    printf ("%i] = %i\n", i, map[i]);
  }

  if (flag) printf ("c perfect hash\n");

}
