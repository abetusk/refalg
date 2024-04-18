#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "rbv.h"

int test0(void) {
  int ret_code=0;
  int32_t i;
  int it=0, n_it=1000;
  int n= 67, stride =4;
  int16_t p;
  int8_t v;

  int16_t x;

  int32_t *check_vec, prv, check_val;

  rbv_t *rbv;

  rbv = rbv_alloc(n);

  check_vec = (int32_t *)malloc(sizeof(int32_t)*(n+1));


  prv = 0;
  check_vec[0]=0;
  for (i=0; i<n; i++) {
    check_val = 0;
    if (rand()%2) {
      rbv_val(rbv, i, 1);

      check_val = 1;
    }
    check_vec[i+1] = check_vec[i] + check_val;
  }
  //rbv_print(rbv);

  for (i=0; i<n; i++) {

    x = rbv_rank_lt(rbv,i);

    //DEBUG
    //printf("[%i] check:%i, rank_lt:%i\n",
    //    i, check_vec[i], x);

    if (check_vec[i] != x) {
      ret_code = -1;
      goto test0_end;
    }
  }


  /*
  x = rbv_rank_lt(rbv, 37);
  printf("got: %i\n", (int)x);

  x = rbv_rank_lt(rbv, 40);
  printf("got: %i\n", (int)x);

  x = rbv_rank_lt(rbv, 48);
  printf("got: %i\n", (int)x);
  */

test0_end:

  rbv_free(rbv);
  free(check_vec);
  return ret_code;
}

int main(int argc, char **argv) {
  int it=0, n_it=1000, r;
  int n= 31, stride =4;
  int16_t p;
  int8_t v;
  rbv_t *rbv;

  r = test0();
  printf("got:%i\n", r);
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
