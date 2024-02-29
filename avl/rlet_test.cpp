//
// To the extent possible under law, the person who associated CC0 with
// this file has waived all copyright and related or neighboring rights
// to this file.
//
// You should have received a copy of the CC0 legalcode along with this
// work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//

#include "rlet.hpp"

int rlet_a_test_0() {
  RLET_A rlet;
  int r, i, n=10000, m = 100, M = 100;
  int64_t idx;
  int32_t val;

  int32_t *ref_a;

  int verbose = 0;

  ref_a = (int32_t *)calloc(m, sizeof(int32_t));
  r = rlet.init(0, m, 0);
  if (r<0) { return -20; }


  for (i=0; i<n; i++) {

    if (verbose) {
      printf("\n\n[%i] tree>>>>>>>\n", i);
      rlet.print();
      printf("[%i] tree<<<<<<<\n\n\n", i);
    }

    idx = rand() % m;
    val = rand() % M;

    if (verbose) {
      printf("[%i] update: v:%i @ idx:%i..\n", i, (int)val, (int)idx);
    }

    ref_a[idx] = val;
    r = rlet.update(idx, val);
    if (r<0) { return -1; }

    if (verbose) {
      printf("\n[%i] tree after update---\n", i);
      rlet.print();
      printf("[%i] ---\n\n", i);
    }


    val = -1;
    r = rlet.read(&val, idx);
    if (r<0) { return -2; }

    if (ref_a[idx] != val) { return -3; }

    r = rlet.consistency();
    if (r < 0) { return r; }

    for (idx=0; idx<m; idx++) {
      val = -1;
      r = rlet.read(&val, idx);
      if (r<0) { return -4; }

      if (ref_a[idx] != val) { return -5; }
    }


  }

  free(ref_a);
  rlet.destroy();

  return 0;
}

int rlet_a_cpy_test_0() {
  RLET_A rlet, doppleganger;
  int r, i, n=10000, m = 100, M = 100;
  int64_t idx;
  int32_t val;

  int32_t *ref_a;

  int verbose = 0;

  ref_a = (int32_t *)calloc(m, sizeof(int32_t));
  r = rlet.init(0, m, 0);
  if (r<0) { return -20; }


  for (i=0; i<n; i++) {

    if (verbose) {
      printf("\n\n[%i] tree>>>>>>>\n", i);
      rlet.print();
      printf("[%i] tree<<<<<<<\n\n\n", i);
    }

    idx = rand() % m;
    val = rand() % M;

    if (verbose) {
      printf("[%i] update: v:%i @ idx:%i..\n", i, (int)val, (int)idx);
    }

    ref_a[idx] = val;
    r = rlet.update(idx, val);
    if (r<0) { return -1; }

    if (verbose) {
      printf("\n[%i] tree after update---\n", i);
      rlet.print();
      printf("[%i] ---\n\n", i);
    }


    val = -1;
    r = rlet.read(&val, idx);
    if (r<0) { return -2; }

    if (ref_a[idx] != val) { return -3; }

    r = rlet.consistency();
    if (r < 0) { return r; }

    doppleganger.copy( &rlet );

    if (doppleganger.consistency() < 0) {


      printf("[i%i]: dopplegnanger.consistency: %i\n",
          i, doppleganger.consistency());

      doppleganger.print();

      return -10;
    }

    for (idx=0; idx<m; idx++) {
      val = -1;
      r = rlet.read(&val, idx);
      if (r<0) { return -4; }
      if (ref_a[idx] != val) { return -5; }

      val = -1;
      r = doppleganger.read(&val, idx);
      if (r<0) { return -6; }
      if (ref_a[idx] != val) { return -7; }

    }

  }

  free(ref_a);
  rlet.destroy();
  doppleganger.destroy();

  return 0;
}



int rlet_slb_test_0() {
  RLET_SLB rlet;
  int r, i, it, n_it=10000, m=100, M=100;
  int32_t idx,
          tidx,
          val;
  int32_t *ref_a;

  ravl_node_t *node;

  int op;
  int verbose = 0;

  ref_a = (int32_t *)malloc(sizeof(int32_t)*m);
  r = rlet.init(0, m);
  if (r<0) { return -1; }

  for (i=0; i<m; i++) { ref_a[i] = i; }

  for (it=0; it<n_it; it++) {

    if (verbose) {
      printf("[%i/%i]\n", it, n_it);
      printf("----------\n");
      rlet.print();
      printf("----------\n\n\n");
    }

    r = rlet.consistency();
    if (r<0) { return -2; }

    for (i=0, idx=0; i<m; i++) {
      if (ref_a[i] < 0) { continue; }
      if (ref_a[i] >= 0) {
        if (!rlet.exists(i)) { return -6; }
        r = rlet.read( &val, idx );
        if (r<0) { return -4; }
        if (val != i) {

          if (verbose) {
            printf("val %i != i %i (idx:%i)\n", val, i, idx);
          }

          return -5;
        }

        r = rlet.index( &tidx, val);
        if (r<0) { return -12; }
        if (tidx != idx) { return -13; }

        idx++;
      }
      else {
        if (rlet.exists(i)) { return -7; }
      }
    }

    if (idx != rlet.count()) { return -30; }

    val = rand()%m;
    op = rand()%2;

    if (verbose) {
      printf("\n===\n");
      printf("  op:%s(%i), val:%i\n",
          (op == 0) ? "add" : "del", op,
          val);
      printf("===\n\n");
    }

    if (op == 0) {

      r = rlet.add(val);
      if (r>0) {
        if (ref_a[val] < 0) { return -8; }
      }
      ref_a[val] = val;

    }

    else if (op == 1) {

      r = rlet.rem(val);
      if (r<0) {
        if (ref_a[val] >= 0) { return -9; }
      }
      ref_a[val] = -1;

      if (verbose) {
        printf("!!!!! remove val:%i\n", val);
        rlet.print();
      }

    }

  }

  rlet.destroy();

  free(ref_a);

  return 0;
}

int rlet_slb_cpy_test_0() {
  RLET_SLB rlet, doppleganger;
  int r, i, it, n_it=10000, m=100, M=100;
  int32_t idx,
          tidx,
          val;
  int32_t *ref_a;

  ravl_node_t *node;

  int op;
  int verbose = 0;

  ref_a = (int32_t *)malloc(sizeof(int32_t)*m);
  r = rlet.init(0, m);
  if (r<0) { return -1; }

  for (i=0; i<m; i++) { ref_a[i] = i; }

  for (it=0; it<n_it; it++) {

    if (verbose) {
      printf("[%i/%i]\n", it, n_it);
      printf("----------\n");
      rlet.print();
      printf("----------\n\n\n");
    }

    r = rlet.consistency();
    if (r<0) { return -2; }

    r = doppleganger.copy(&rlet);
    if (r<0) { return -20; }

    for (i=0, idx=0; i<m; i++) {

      if (ref_a[i] < 0) { continue; }

      if (ref_a[i] >= 0) {

        if (!rlet.exists(i)) { return -6; }
        r = rlet.read( &val, idx );
        if (r<0) { return -4; }
        if (val != i) {

          if (verbose) {
            printf("val %i != i %i (idx:%i)\n", val, i, idx);
          }

          return -5;
        }

        r = rlet.index( &tidx, val);
        if (r<0) { return -12; }
        if (tidx != idx) { return -13; }

        if (!doppleganger.exists(i)) { return -26; }
        r = doppleganger.read( &val, idx );
        if (r<0) { return -24; }
        if (val != i) {

          if (verbose) {
            printf("doppleganger val %i != i %i (idx:%i)\n", val, i, idx);
          }

          return -25;
        }

        r = doppleganger.index( &tidx, val);
        if (r<0) { return -28; }
        if (tidx != idx) { return -29; }

        idx++;

      }
      else {
        if (rlet.exists(i)) { return -7; }
        if (doppleganger.exists(i)) { return -27; }
      }

    }

    if (idx != rlet.count()) { return -30; }
    if (rlet.count() != doppleganger.count()) { return -31; }


    val = rand()%m;
    op = rand()%2;

    if (verbose) {
      printf("\n===\n");
      printf("  op:%s(%i), val:%i\n",
          (op == 0) ? "add" : "del", op,
          val);
      printf("===\n\n");
    }

    if (op == 0) {

      r = rlet.add(val);
      if (r>0) {
        if (ref_a[val] < 0) { return -8; }
      }
      ref_a[val] = val;

    }

    else if (op == 1) {

      r = rlet.rem(val);
      if (r<0) {
        if (ref_a[val] >= 0) { return -9; }
      }
      ref_a[val] = -1;

      if (verbose) {
        printf("!!!!! remove val:%i\n", val);
        rlet.print();
      }

    }

  }

  rlet.destroy();
  doppleganger.destroy();

  free(ref_a);

  return 0;
}

int main(int argc, char **argv) {
  int r;

  r = rlet_a_test_0();
  printf("rlet_a_test_0: %i\n", r);

  r = rlet_a_cpy_test_0();
  printf("rlet_a_cpy_test_0: %i\n", r);

  printf("---\n\n");

  r = rlet_slb_test_0();
  printf("rlet_slb_test_0: %i\n", r);

  r = rlet_slb_cpy_test_0();
  printf("rlet_slb_cpy_test_0: %i\n", r);

  return 0;
}
