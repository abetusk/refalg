Rank Bit Vector
===

Small reference implementation of ranked bit vector.

This stores a bit vector and supporting structure to do rank queries in $O(\lg n)$ time.

Estimates on space is $\frac{3.5 \cdot n}{8}$.
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
```


