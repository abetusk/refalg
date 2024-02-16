//
// To the extent possible under law, the person who associated CC0 with
// this file has waived all copyright and related or neighboring rights
// to this file.
//
// You should have received a copy of the CC0 legalcode along with this
// work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//

#include "rlet.hpp"

int rlet_test_0() {
  RLET_A rlet;
  int r, i, n=1000, m = 100, M = 100;
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

  }

  for (idx=0; idx<m; idx++) {
    val = -1;
    r = rlet.read(&val, idx);
    if (r<0) { return -4; }

    if (ref_a[idx] != val) { return -5; }
  }

  free(ref_a);

  rlet.destroy();

  return 0;
}

int main(int argc, char **argv) {
  int r;
  RLET_A rlet;

  r = rlet_test_0();

  printf("...%i\n", r);
}
