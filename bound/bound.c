#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char** argv) {
  int i, n = atoi (argv[1]);
  long double s = 1.0, t = 0.0;

  for (i = 1; i <= n; i++) {
    s *= (long double) n / (long double) i;
    while (s > 100) { t += 5; s /= 32; }
  }

  s = log(s) / log (2);

  printf ("%.3Lf\n", (s+t) / n);
}
