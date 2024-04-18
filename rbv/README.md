Rank Bit Vector
===

Small reference implementation of ranked bit vector.

This stores a bit vector and supporting structure to do rank queries in $O(\lg n)$ time.

An estimate on space is $\frac{3.5 \cdot n}{8}$.
`int16_t` are hard coded and the maximum bit vector allowable is 32768.

The relevant API is:

| Function | Description |
|---|---|
| `rbv_alloc(n)` | Allocate a bit vector of $n$ bits |
| `rbv_free(rbv)` | Free allocated `rbv_t` structure |
| `rbv_val(rbv, pos, val)` | If `val` is `{0,1}`, set bit at `pos` to `val`. Return value at `pos` in bit vector |
| `rbv_rank_lt(rbv, pos)` | Return number of set bits (rank) up to but not including `pos` |
| `rbv_rank(rbv, s, e)` | Return number of set bits (rank) from start `s` (inclusive) to end `e` (non-inclusive) |
| `rbv_rank_idx(rbv, rank)` | Return index of `rank` set bit |

Example
---

```
#include "rbv.h"

int main(int argc, char **argv) {
  int i;
  int16_t t, m;
  rbv_t *rbv;

  rbv = rbv_alloc(137);

  for (i=0; i<137; i++) {
    if (rand()%2) { rbv_val(rbv, i, 1); }
    else          { rbv_val(rbv, i, 0); }
  }

  t = rbv_rank_lt(rbv, 55);
  printf("rank %i @ %i\n", t, 55);

  t = rbv_rank(rbv, 33, 55);
  printf("rank %i from [%i,%i)\n", t, 33,55);

  m = rbv_rank_lt(rbv, 137);
  printf("total rank %i\n", m);

  t = rbv_rank_idx(rbv, m/2);
  printf("rank %i has bit position %i\n", m/2, t);

  rbv_free(rbv);
}
```

```
$ gcc rbv.c example.c -o example
```

License
---

Creative Commons 0 (CC0) 

![cc0](../img/cc0_88x31.png)

