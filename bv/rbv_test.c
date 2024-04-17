#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "rbv.h"

int test0(void) {
  int32_t i;
  int it=0, n_it=1000;
  int n= 31, stride =4;
  int16_t p;
  int8_t v;

  int16_t x;

  rbv_t *rbv;

  rbv = rbv_alloc(n);

  for (i=0; i<n; i++) {
    if (rand()%2) {
      rbv_val(rbv, i, 1);
    }
  }
  rbv_print(rbv);

  x = rbv_rank(rbv, 7, 17);

  printf("got: %i\n", (int)x);

  rbv_free(rbv);
  return 0;
}

int main(int argc, char **argv) {
  int it=0, n_it=1000;
  int n= 31, stride =4;
  int16_t p;
  int8_t v;
  rbv_t *rbv;

  test0();
  exit(0);

  if (argc > 1) {
    n = atoi(argv[1]);
  }

  rbv = rbv_alloc(n);

  for (it=0; it<n_it; it++) {
    printf("\n\n");
    p = rand()%n;
    v = rand()%2;

    rbv_val(rbv, p, v);

    printf("[%i/%i] p:%i, v:%i\n", it, n_it, (int)p, (int)v);
    rbv_print(rbv);

    printf("sanity:%i\n", rbv_sanity(rbv));

  }
  printf("\n");

  /*
  p = 17; v = 1;
  printf("[*] p:%i, v:%i\n", (int)p, (int)v);
  rbv_val(rbv, p, v);
  rbv_print(rbv);

  printf("\n");
  */



  rbv_free(rbv);
}
