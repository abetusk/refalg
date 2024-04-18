#include "rbv.h"

static int _popcount8(uint8_t u8) {
  int i;
  int8_t c=0;
  for (i=0; i<8; i++) {
    c += ((u8&1)?1:0);
    u8 /= 2;
  }
  return c;
}

static int _popcount8_se(uint8_t u8, uint8_t s, uint8_t e) {
  int i;
  int8_t c=0;
  for (i=s; i<e; i++) {
    c += ((u8&1)?1:0);
    u8 /= 2;
  }
  return c;
}

rbv_t *rbv_alloc(int16_t n) {
  int32_t i;
  rbv_t *rbv;

  rbv = (rbv_t *)malloc(sizeof(rbv_t));

  rbv->n = n;
  rbv->stride = 8;

  rbv->n_bv = (int16_t)(n/8);
  if ((n%8) != 0) { rbv->n_bv++; }

  rbv->n_limb = n/rbv->stride;
  if ((n%(rbv->stride)) != 0) { rbv->n_limb++; }

  rbv->n_rank = 1;
  for (i=0; i<15; i++) {
    if (rbv->n_rank >= rbv->n_limb) { break; }
    rbv->n_rank *= 2;
  }
  rbv->n_rank--;
  
  rbv->bv = (uint8_t *)malloc(sizeof(uint8_t)*(rbv->n_bv));
  rbv->limb = (int16_t *)malloc(sizeof(int16_t)*(rbv->n_limb));
  rbv->rank = (int16_t *)malloc(sizeof(int16_t)*(rbv->n_rank));

  memset(rbv->bv, 0, sizeof(uint8_t)*(rbv->n_bv));
  memset(rbv->limb, 0, sizeof(uint16_t)*(rbv->n_limb));
  memset(rbv->rank, 0, sizeof(uint16_t)*(rbv->n_rank));

  rbv->lookup = (int16_t *)malloc(sizeof(int16_t)*(1<<(rbv->stride)));

  for (i=0; i<(1<<(rbv->stride)); i++) {
    rbv->lookup[i] = _popcount8((uint8_t)i);
  }

  return rbv;
}

/*
rbv_t *rbv_alloc(int16_t n, uint8_t stride) {
  int32_t i;
  rbv_t *rbv;

  rbv = (rbv_t *)malloc(sizeof(rbv_t));

  rbv->n = n;
  rbv->stride = stride;

  rbv->n_bv = (int16_t)(n/8);
  if ((n%8) != 0) { rbv->n_bv++; }

  rbv->n_limb = n/stride;
  if ((n%stride) != 0) { rbv->n_limb++; }

  rbv->n_rank = 1;
  for (i=0; i<15; i++) {
    if (rbv->n_rank > rbv->n_limb) { break; }
    rbv->n_rank *= 2;
  }
  rbv->n_rank--;
  
  rbv->bv = (uint8_t *)malloc(sizeof(uint8_t)*(rbv->n_bv));
  rbv->limb = (int16_t *)malloc(sizeof(int16_t)*(rbv->n_limb));
  rbv->rank = (int16_t *)malloc(sizeof(int16_t)*(rbv->n_rank));

  memset(rbv->bv, 0, sizeof(uint8_t)*(rbv->n_bv));
  memset(rbv->limb, 0, sizeof(uint16_t)*(rbv->n_limb));
  memset(rbv->rank, 0, sizeof(uint16_t)*(rbv->n_rank));

  rbv->lookup = (int16_t *)malloc(sizeof(int16_t)*(1<<stride));

  for (i=0; i<(1<<stride); i++) {
    rbv->lookup[i] = _popcount8((uint8_t)i);
  }

  return rbv;
}
*/

void rbv_free(rbv_t *rbv) {
  if (!rbv) { return; }
  if (rbv->bv) { free(rbv->bv); }
  if (rbv->limb) { free(rbv->limb); }
  if (rbv->rank) { free(rbv->rank); }
  if (rbv->lookup) { free(rbv->lookup); }
  free(rbv);
}

int16_t rbv_rank_lt(rbv_t *rbv, int32_t p) {
  uint8_t rem, mask, u8;
  int16_t bv_idx,
          limb_idx,
          rank_idx,
          par_idx;
  int16_t sum=0;

  bv_idx = p / 8;
  limb_idx = p / 8;
  rank_idx = (rbv->n_rank/2) + (limb_idx/2);

  if ((limb_idx%2) == 1) {
    sum = rbv->limb[limb_idx-1];
  }

  //DEBUG
  //printf("rank_idx:%i\n", rank_idx);

  while (rank_idx > 0) {
    par_idx = (rank_idx-1) / 2;

    //DEBUG
    //printf("..rank_idx:%i, par_idx:%i\n", rank_idx, par_idx);

    if (((2*par_idx)+1) != rank_idx) {

      //DEBUG
      //printf("...rank[par:%i]:%i - rank[idx:%i]:%i (prvsum:%i)\n",
      //    par_idx, rbv->rank[par_idx], rank_idx, rbv->rank[rank_idx], sum);

      sum += rbv->rank[par_idx] - rbv->rank[rank_idx];
    }

    rank_idx = (rank_idx-1)/2;
  }

  rem = p%8;
  if (rem != 0) {

    mask = ((uint8_t)0xff) >> ((uint8_t)(8-rem)) ;
    u8 = rbv->bv[ bv_idx ];


    //printf(">>lookup[%i] %i\n",
    //    mask & u8,
    //    rbv->lookup[ mask & u8 ]);


    //printf("  rbv->bv[%i]: %02x, masked:%02x\n",
    //    bv_idx, u8, u8&mask);

    sum += rbv->lookup[ u8 & mask ];
  }

  return sum;
}

// return population count from start to end, non inclusive
//
// returns -1 on error
//
int16_t rbv_rank(rbv_t *rbv, int32_t s, int32_t e) {
  int16_t s_s, s_e;

  s_s = rbv_rank_lt(rbv, s);
  s_e = rbv_rank_lt(rbv, e);

  return s_e - s_s;
}

// If val = {0,1}, update bit vector position with value val
// return value at bit vector position.
//
// Updates limb and rank arrays
//
uint8_t rbv_val(rbv_t *rbv, int16_t pos, int8_t val) {
  int16_t base_idx, offset,
          limb_idx,
          rank_idx;
  uint8_t u8, v8, t8;
  int8_t tv, dv;

  base_idx = pos / 8;
  offset = pos % 8;

  // save old value and if its the same, return
  // the value without needing to write.
  //
  u8 = rbv->bv[base_idx];
  tv = (u8 & (1<<offset)) ? 1 : 0;
  if ((val < 0) || (val > 1)) { return tv; }
  if (tv==val) { return tv; }

  // otherwise, write to bv array
  //
  if (val==0) {
    u8 &= ~((uint8_t)1<<offset);
    rbv->bv[base_idx] = u8;
  }
  else {
    u8 |= ((uint8_t)1<<offset);
    rbv->bv[base_idx] = u8;
  }

  // rank must change since the value cahnged,
  // so increment/decreemnt accordingly
  //
  dv = ((val==0) ? -1 : 1);
  limb_idx = pos / rbv->stride;
  rbv->limb[limb_idx] += dv;

  rank_idx = rbv->n_rank/2;
  rank_idx += limb_idx/2;

  while (rank_idx>0) {
    rbv->rank[rank_idx] += dv;
    rank_idx--;
    rank_idx /= 2;
  }
  rbv->rank[rank_idx] += dv;

  return val;
}

int16_t rbv_pos(rbv_t *rbv, int16_t idx) {
  return -1;
}

int rbv_sanity(rbv_t *rbv) {
  int32_t i, j, k;
  int32_t count, c;
  int16_t idx, limb_idx;
  int l, r;

  uint8_t u8, v8;

  count = 0;
  for (i=0; i<rbv->n; i++) {
    idx = i/8;

    u8 = rbv->bv[idx];

    count += ((u8 & (1<<(i%8))) ? 1 : 0);

    if ((i>0) &&
        (((i+1)%rbv->stride)==0)) {
      limb_idx = i/rbv->stride;
      if (count != rbv->limb[limb_idx]) { return -1; }
      count=0;
    }

  }
  if ((rbv->n % rbv->stride) != 0) {
    limb_idx = (rbv->n / rbv->stride);
    if (count != rbv->limb[limb_idx]) { return -1; }
  }

  for (i=0; i<(rbv->n_rank/2); i++) {
    l = 2*i+1;
    r = 2*i+2;

    c = 0;
    if (l < rbv->n_rank) { c += rbv->rank[l]; }
    if (r < rbv->n_rank) { c += rbv->rank[r]; }

    if (c != rbv->rank[i]) { return -3; }
  }

  for (i=(rbv->n_rank/2); i<rbv->n_rank; i++) {
    c = 0;

    limb_idx = 2*(i-(rbv->n_rank/2))+0;
    if (limb_idx < rbv->n_limb) { c += rbv->limb[limb_idx]; }

    limb_idx = 2*(i-(rbv->n_rank/2))+1;
    if (limb_idx < rbv->n_limb) { c += rbv->limb[limb_idx]; }

    if (c != rbv->rank[i]) { return -4; }
  }

  for (i=0; i<(1<<(rbv->stride)); i++) {
    if (rbv->lookup[i] != _popcount8((uint8_t)i)) { return -5; }
  }

  return 0;
}

void rbv_print(rbv_t *rbv) {
  int i;
  int disp_w = 16;

  int64_t pow2=2;

  printf("n: %i\n", (int)rbv->n);
  printf("stride: %i\n", (int)rbv->stride);
  printf("n_bv: %i\n", (int)rbv->n_bv);
  printf("n_limb: %i\n", (int)rbv->n_limb);

  printf("n_rank: %i\n", (int)rbv->n_rank);

  printf("bv[%i]:", (int)rbv->n_bv);
  for (i=0; i<rbv->n_bv; i++) {
    if ((i%disp_w)==0) { printf("\n"); }
    printf(" %02x", (int)rbv->bv[i]);
  }
  printf("\n");

  printf("limb[%i]:", (int)rbv->n_limb);
  for (i=0; i<rbv->n_limb; i++) {
    if ((i%disp_w)==0) { printf("\n"); }
    printf(" %02x", (int)rbv->limb[i]);
  }
  printf("\n");

  printf("rank[%i]:\n", (int)rbv->n_rank);
  for (i=0; i<rbv->n_rank; i++) {
    if (i==(pow2-1)) {
      printf(" |\n");
      pow2*=2;
    }
    //if ((i%disp_w)==0) { printf("\n"); }
    printf(" %02x", (int)rbv->rank[i]);
  }
  printf("\n");
}

//----
//----
//----
//----

/*
int main(int argc, char **argv) {
  rbv_t *rbv;

  rbv = rbv_alloc(17, 4);

  rbv_print(rbv);

  rbv_free(rbv);
}
*/