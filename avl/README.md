Reference AVL Implementation
===

This is a basic unoptimized implementation of an AVL tree.

C++ is used but is used sparsely.
This should make it a bit easier to convert to C if desired.

The major difference between this implementation and others
is the `m_update` function, allowing for nodes that have been
altered (rotated, updated, etc.) to have a callback associated
with them.
This includes nodes that need to go up the chain to alter their
ancestors.

Two other data structures are implemented that build off of the
AVL implementation.

Run Length Encoded Tree Array
===

This data structure acts as a fixed length `int32_t` array.

After `init` is called, any element in the virtual array
can be updated or retrieved.

Each node stores an interval and value, allowing only
one node to represent a whole interval of values.

The benefits of such a structure are:

* Sparse (space efficient) representation of an array that
  has many identical contiguous elements
* $O(\lg N)$ lookups and updates, inheriting the run-time
  efficiency of a self balancing tree

The cons:

* If the array is not sparse, this will be memory inefficient
  - in the worst case, a "checkerboard" pattern will cause
    maximum inefficiency, requiring a node allocation for
    each element in the array
* $O(\lg N)$ lookups and updates, which can be much slower
  than $O(1)$ lookups and updates, depending on the application


Run Length Encoded Tree, Sparse Linear Biased
===

This data structure acts as a "set"-like structure for a
contiguous range of `int32_t` elements, with each element
appearing either none or one times.

This is meant to be used as if it held elements from `[0,1, ... ,(m-1)]`
which can be removed, added back in or tested for inclusion.
Further, index operations can be performed, allowing for the efficient
($O(\lg N)$) retrieval of any element, referenced by its index rather
than its value.

The index operations allow us to enumerate the elements in $O(N \lg N)$
time, so long as the structure is static during that time.

The benefits:

* Sparse (space efficient) representation of subsets of `[0,1, ... ,(m-1)]`
* $O(\lg N)$ lookups by value or by index
* $O(\lg N)$ insertion and removal

The cons:

* Large memory footprint if data is not sparse
  - In the worst case, if every other element is removed, the resulting
    "checkerboard" pattern of remaining elements will cause the data
    structure to have a large memory footprint
* $O(\lg N)$ operations (lookup by value, by index, insertion, removal)
  which can be much slower than an $O(1)$ runtime

If, for example, one has an array of 10k elements, but, over the course
of its lifetime it starts out almost completely populated and then becomes
sparsely populated, perhaps with 1-100 elements,
this data structure could help.


