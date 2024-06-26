// To the extent possible under law, the person who associated CC0 with
// this file has waived all copyright and related or neighboring rights
// to this file.
//     
// You should have received a copy of the CC0 legalcode along with this
// work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <sys/time.h>

#include "rbv.h"

//#define RBV_MAIN_DEBUG 1

//---
//---

int test_bitcheck(int n) {
  int ret_code=0;
  int32_t i;
  int32_t *check_vec;
  int16_t x;

  rbv_t *rbv;

  if (n<0) { n = 67; }

  rbv = rbv_alloc(n);
  check_vec = (int *)malloc(sizeof(int)*n);

  for (i=0; i<n; i++) {
    if (rand()%2) {
      rbv_val(rbv, i, 1);
      check_vec[i] = 1;
    }
    else {
      rbv_val(rbv, i, 0);
      check_vec[i] = 0;
    }
  }

  for (i=0; i<n; i++) {
    if (check_vec[i] != rbv_val(rbv, i, -1)) {
      ret_code = -1;
      goto test_bitcheck_end;
    }
  }


test_bitcheck_end:

  free(check_vec);
  rbv_free(rbv);

  return ret_code;
}

int test0(void) {
  int ret_code=0;
  int n= 67;
  int32_t i;
  int32_t *check_vec, check_val;
  int16_t x;
  rbv_t *rbv;

  rbv = rbv_alloc(n);
  check_vec = (int32_t *)malloc(sizeof(int32_t)*(n+1));

  // setup
  //
  check_vec[0]=0;
  for (i=0; i<n; i++) {
    check_val = 0;
    if (rand()%2) {
      rbv_val(rbv, i, 1);
      check_val = 1;
    }
    check_vec[i+1] = check_vec[i] + check_val;
  }

  // check
  //
  for (i=0; i<n; i++) {
    x = rbv_rank_lt(rbv,i);
    if (check_vec[i] != x) {
      ret_code = -1;
      goto test0_end;
    }
  }

test0_end:

  rbv_free(rbv);
  free(check_vec);
  return ret_code;
}

int test_rank_idx(int n) {
  int ret_code = 0;
  int32_t i;
  int32_t *check_vec, check_val;
  int16_t x, count=0;
  rbv_t *rbv;

  if (n<0) { n = 129; }

  rbv = rbv_alloc(n);
  check_vec = (int32_t *)malloc(sizeof(int32_t)*(n+1));

  // setup
  //
  check_vec[0]=0;
  for (i=0; i<n; i++) {
    if (rand()%2) {
      rbv_val(rbv, i, 1);
      count++;
      check_vec[count] = i;
    }
  }

  // check
  //
  for (i=1; i<count; i++) {

    //printf("###i:%i\n", i);
    //rbv_print(rbv);

    x = rbv_rank_idx(rbv,i);
    if (check_vec[i] != x) {

      printf("mismatch, i:%i, got:%i, expected:%i\n",
          i, x, check_vec[i]);

      ret_code = -1;
      goto test_rank_idx_end;
    }

    //printf("i:%i ok (%i==%i)\n", i, x, check_vec[i]);
  }

test_rank_idx_end:

  free(check_vec);
  rbv_free(rbv);

  return ret_code;
}

//---
//---

int test_n_p(int n, double p) {
  int ret_code=0;
  int32_t i, j;
  int32_t *check_vec, check_val;
  int16_t x;

  double q;
  rbv_t *rbv;

  rbv = rbv_alloc(n);
  check_vec = (int32_t *)malloc(sizeof(int32_t)*(n+1));

  // setup
  //
  check_vec[0]=0;
  for (i=0; i<n; i++) {
    check_val = 0;

    q = rand();
    q /= (double)(RAND_MAX + 1.0);

    if (q < p) {
      rbv_val(rbv, i, 1);
      check_val = 1;
    }
    check_vec[i+1] = check_vec[i] + check_val;
  }

#ifdef RBV_MAIN_DEBUG
  rbv_print(rbv);
#endif



  // check
  //
  for (i=0; i<=n; i++) {
    x = rbv_rank_lt(rbv,i);
    if (check_vec[i] != x) {
      ret_code = -1;
      goto test_n_p_end;
    }
  }

  for (i=0; i<=n; i++) {
    for (j=i; j<=n; j++) {
      x = rbv_rank(rbv, i, j);
      if ((check_vec[j]-check_vec[i]) != x) {
        ret_code = -1;
        goto test_n_p_end;
      }
    }
  }

test_n_p_end:

  rbv_free(rbv);
  free(check_vec);
  return ret_code;
}

//---
//---


int test_n_p_prof(int n, double p, int n_it) {
  int ret_code=0;
  int16_t s,e;
  int32_t i;
  int16_t x;
  double q;

  int64_t v_a, v_b, it;
  double d;

  struct timeval tv_s, tv_e;

  rbv_t *rbv;

  rbv = rbv_alloc(n);

  // setup
  //
  for (i=0; i<n; i++) {

    q = rand();
    q /= (double)(RAND_MAX + 1.0);

    if (q < p) {
      rbv_val(rbv, i, 1);
    }
  }

  //rbv_print(rbv);

  // timing
  //
  v_a=0;

  gettimeofday(&tv_s, NULL);
  for (it=0; it<n_it; it++) {

    s = rand()%n;
    e = rand()%(n-s) + s;

    v_a += rbv_rank(rbv, s, e);
  }
  gettimeofday(&tv_e, NULL);

  d = (double)tv_e.tv_sec + ((double)tv_e.tv_usec)/1000000.0;
  d -= (double)tv_s.tv_sec + ((double)tv_s.tv_usec)/1000000.0;
  printf("rank time: %f\n", d);

  rbv_free(rbv);
  return ret_code;
}

//---
//---

int test_n_p_rt(int n, double p, int n_it) {
  int ret_code=0;
  int16_t s,e;
  int32_t i;
  int16_t x;
  double q;

  int64_t v_a, v_b, it;
  double d;

  struct timeval tv_s, tv_e;

  rbv_t *rbv;

  rbv = rbv_alloc(n);

  // setup
  //
  for (i=0; i<n; i++) {

    q = rand();
    q /= (double)(RAND_MAX + 1.0);

    if (q < p) {
      rbv_val(rbv, i, 1);
    }
  }

  //rbv_print(rbv);

  // timing
  //
  v_a=0;

  gettimeofday(&tv_s, NULL);
  for (it=0; it<n_it; it++) {

    s = rand()%n;
    e = rand()%(n-s) + s;

    v_a += rbv_rank(rbv, s, e);
  }
  gettimeofday(&tv_e, NULL);

  d = (double)tv_e.tv_sec + ((double)tv_e.tv_usec)/1000000.0;
  d -= (double)tv_s.tv_sec + ((double)tv_s.tv_usec)/1000000.0;
  printf("rank time: %f (%i)\n", d, (int)v_a);


  v_b = 0;
  gettimeofday(&tv_s, NULL);
  for (it=0; it<n_it; it++) {

    s = rand()%n;
    e = rand()%(n-s) + s;

    v_b += rbv_rank_lin(rbv, s, e);
  }
  gettimeofday(&tv_e, NULL);

  d = (double)tv_e.tv_sec + ((double)tv_e.tv_usec)/1000000.0;
  d -= (double)tv_s.tv_sec + ((double)tv_s.tv_usec)/1000000.0;
  printf("lin time: %f (%i)\n", d, (int)v_b);

  rbv_free(rbv);
  return ret_code;
}

//---
//---

int main(int argc, char **argv) {
  int i;
  int it=0, n_it=1000, r;
  int n= 31, stride =4;
  int16_t p;
  int8_t v;
  rbv_t *rbv;

  r = test0();
  printf("# test0: %s\n", (r==0) ? "pass" : "ERROR");
  if (r<0) { exit(-1); }

  r = test_bitcheck(-1);
  printf("# test_bitcheck: %s\n", (r==0) ? "pass" : "ERROR");
  if (r<0) { exit(-1); }

  r = test_n_p(11979,0.5);
  printf("# test_n_p(11979,0.5): %s\n", (r==0) ? "pass" : "ERROR");
  if (r<0) { exit(-1); }

  for (i=1; i<33; i++) {
    r = test_n_p(i,0.5);
    printf("# test_n_p(%i,0.5): %s\n", i, (r==0) ? "pass" : "ERROR");
    if (r<0) { exit(-1); }
  }


  r = test_rank_idx(-1);
  printf("# test_rank_idx: %s\n", (r==0) ? "pass" : "ERROR");
  if (r<0) { exit(-1); }


  //test_n_p_prof(11979,0.5, 10000000);

  exit(0);
}
