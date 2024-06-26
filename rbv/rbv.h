// To the extent possible under law, the person who associated CC0 with
// this file has waived all copyright and related or neighboring rights
// to this file.
//     
// You should have received a copy of the CC0 legalcode along with this
// work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//
#ifndef RANK_BITVEC_HEADER
#define RANK_BITVEC_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct rbv16_s8_type {
  int16_t n,
          n_bv,
          n_rank,
          n_limb;
  uint8_t stride;

  uint8_t *bv;
  int16_t *rank;
  int16_t *limb;

  int16_t *lookup;
} rbv16_s8_t;

typedef struct rbv16_s8_type rbv_t;

//rbv_t *rbv_alloc(int16_t n, uint8_t stride);
rbv_t *rbv_alloc(int16_t n);
void rbv_free(rbv_t *rbv);

int16_t rbv_rank_lin(rbv_t *rbv, int32_t s, int32_t e);

int16_t rbv_rank_lt(rbv_t *rbv, int32_t p);
int16_t rbv_rank(rbv_t *rbv, int32_t s, int32_t e);
uint8_t rbv_val(rbv_t *rbv, int16_t pos, int8_t val);
int16_t rbv_rank_idx(rbv_t *rbv, int16_t query_rank);

int rbv_sanity(rbv_t *rbv);
void rbv_print(rbv_t *rbv);

#endif
