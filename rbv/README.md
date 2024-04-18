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

  t = rbv_val(rbv, 55, -1);
  printf("bit is %i @ position %i\n", t, 55);

  t = rbv_rank_idx(rbv, m/2);
  printf("rank %i has bit position %i\n", m/2, t);

  rbv_free(rbv);
}
```

```
$ gcc rbv.c example.c -o example
```

Implementation Details
---

There are three main structures:

* `bv` : the bit vector holding the underlying data, stored as `uint8_t`
* `limb`: sum of  a `stride`s worth of bits, currently hard coded to `8` (1 byte of `bv` per limb)
* `rank`: a heap-like tree that holds the rank of the sub tree below it

`limb` can be regarded as the leaf node to the `rank` heap-like tree but `limb` has no `0` padding at the end,
whereas the `rank` array is of size $2^{s}-1$ and padded with zeros appropriately.

Setting bits in `bv` results in the `limb` being updated and tracing the updating up the `rank` array
until it reaches the root.

Rank queries are performed by descending the `rank` array-tree, keeping appropriate state as the descent
occurs.
When getting to the end, a stride scan of the bit vector element is done to find the last position.

`rbv_rank()` is implemented as two calls to `rbv_rank_lt()` with the appropriate bounds.

License
---

Creative Commons 0 (CC0) 

![cc0](../img/cc0_88x31.png)

