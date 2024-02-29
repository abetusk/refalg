//
// To the extent possible under law, the person who associated CC0 with
// this file has waived all copyright and related or neighboring rights
// to this file.
//
// You should have received a copy of the CC0 legalcode along with this
// work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//

#ifndef RLET_HPP
#define RLET_HPP

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ravl.hpp"

typedef struct value_interval_range_type {
  int32_t v;
  int64_t s, e;
} value_range_t;

static value_range_t *_vr_create(int32_t v, int64_t s, int64_t e) {
  value_range_t *vr;
  vr = (value_range_t *)malloc(sizeof(value_range_t));
  vr->v = v;
  vr->s = s;
  vr->e = e;
  return vr;
}

static int _vr_cmp(void *_a, void *_b) {
  value_range_t *a, *b;

  a = (value_range_t *)_a;
  b = (value_range_t *)_b;

  if (a->s < b->s) { return -1; }
  if (a->s > b->s) { return  1; }

  return 0;
}

static void _vr_free(void *_a) { free(_a); }

static void _vr_print(void *_a) {
  value_range_t *a;
  a = (value_range_t *)_a;
  printf("[%i:%i]{v:%i}\n",
      (int)a->s, (int)a->e, (int)a->v);
}

static void _node_print(ravl_node_t *node) {
  _vr_print(node->data);
}

static void _vr_update(ravl_node_t *node) {
}

// run length encoded tree - fixed array 
//
// Nodes in the internal balanced tree (`m_tree`)
// represent a run of value, `v` from start `s`
// to `e` non inclusive.
// The object must be initialized before use with
// the start, end and initial value in the virutal array.
//
// Internally, each node reprsents a disjoint and non-overlapping
// interval associated with each value.
// Intervals are non overlapping but contiguous.
//
// The only operation currently supported is inserting
// a value that is unit interval long.
// A query is created with the start of the interval as key
// and searched.
// If the query interval is found and the values match,
// not further processing needs to be done.
// If the value is different, the interval is split,
// inserting a new interval in the middle and removing
// the left and right split intervals as necessary if
// the query interval falls on the endpoints.
// If the newly inserted interval neighbors intervals with
// the same value, those intervals are merged by extending
// the new interval and deleting the neighboring entries.
//
// The main workflow is:
//
// T.init(s,e)
// ...
// T.update(idx,val)
// T.read(&val, idx);
// ...
// T.destroy()
//
//
class RLET_A {
  public:

    RLET_A() {
      m_tree.m_cmp    = _vr_cmp;
      m_tree.m_free   = _vr_free;
      m_tree.m_node_print  = _node_print;
      m_tree.m_update = _vr_update;

      m_node_count = 0;
      m_start = 0;
      m_length = 0;
    }
    ~RLET_A() {}

    void destroy() { m_tree.destroy(); }

    void print() {
      printf("m_tree: node_count:%i, start:%i, length:%i\n",
          (int)m_node_count, (int)m_start, (int)m_length);
      m_tree.print_tree();
    }

    int init(int64_t s, int64_t e_ninc, int32_t val) {
      value_range_t *vr;

      m_tree.destroy();
      m_node_count=1;
      vr = _vr_create(val, s, e_ninc);

      return m_tree.add(vr);
    }

    int copy_r(ravl_node_type *src_node) {
      int r;
      value_range_t *src_data, *dst_data;

      if (!src_node) { return 0; }

      dst_data = NULL;
      if (src_node->l) {
        src_data = (value_range_t *)(src_node->l->data);
        dst_data = _vr_create(src_data->v, src_data->s, src_data->e);

        r = m_tree.add(dst_data);
        if (r<0) { return r; }
      }

      dst_data = NULL;
      if (src_node->r) {
        src_data = (value_range_t *)(src_node->r->data);
        dst_data = _vr_create(src_data->v, src_data->s, src_data->e);

        r = m_tree.add(dst_data);
        if (r<0) { return r; }
      }

      r = copy_r(src_node->l);
      if (r<0) { return r; }

      r = copy_r(src_node->r);
      if (r<0) { return r; }

      return 0;
    }

    // functional copy
    //
    int copy(RLET_A *src) {
      int r;
      value_range_t *src_data, *dst_data;

      m_tree.destroy();

      m_node_count  = src->m_node_count;
      m_start       = src->m_start;
      m_length      = src->m_length;

      if (src->m_tree.m_root) {
        src_data = (value_range_t *)(src->m_tree.m_root->data);
        dst_data = _vr_create(src_data->v, src_data->s, src_data->e);

        r = m_tree.add(dst_data);
        if (r<0) { return r; }
      }

      r = copy_r(src->m_tree.m_root);
      if (r<0) { return r; }

      return 0;
    }



    int consistency_r(ravl_node_t *node, int lvl=0) {
      int r;
      value_range_t *vr, *vr_l, *vr_r;
      ravl_node_t *node_l,
                  *node_r;

      if (!node) { return 0; }

      vr = (value_range_t *)(node->data);
      if (!vr) { return -1; }

      node_l = m_tree.pred(node);
      if (node_l) {
        vr_l = (value_range_t *)(node_l->data);
        if (!vr_l) { return -2; }
        if (vr_l->v == vr->v) { return -4; }
        if (vr_l->e != vr->s) { return -6; }
      }

      node_r = m_tree.succ(node);
      if (node_r) {
        vr_r = (value_range_t *)(node_r->data);
        if (!vr_r) { return -3; }
        if (vr_r->v == vr->v) { return -5; }
        if (vr_r->s != vr->e) { return -7; }
      }

      node_l = node->l;
      if (node_l) {
        r = consistency_r(node_l, lvl+1);
        if (r<0) { return r; }
      }

      node_r = node->r;
      if (node_r) {
        r = consistency_r(node_r, lvl+1);
        if (r<0) { return r; }
      }

      return 0;
    }

    int consistency(void) {
      int r = 0;
      r = consistency_r(m_tree.m_root, 0);
      return r;
    }

    // get value at virtual index.
    // return:
    //
    //  0  : success, *val populated with value
    // -1  : error, index out of bounds
    //
    int read(int32_t *val, int64_t index) {
      value_range_t *vr;
      ravl_node_t *node;

      if (!m_tree.m_root) { return -1; }

      node = m_tree.m_root;
      while (node) {
        vr = (value_range_t *)(node->data);

        if ((vr->s <= index) &&
            (index <  vr->e)) {
          if (val) { *val = vr->v; }
          return 0;
        }

        if (vr->e <= index) { node = node->r; continue; }
        if (vr->s >  index) { node = node->l; continue; }
        return -1;
      }

      return -1;
    }

    // update implicit array with val at index.
    // return:
    //
    // We have to be careful about order of operations
    // as the balanced tree keys off of the start of
    // the interval. If there are duplicate keys,
    // we might have nodes returned to us that are
    // not the expected ones.
    //
    // -1  :  error
    //  0  :  success
    //  1  :  element already exists (nothing done)
    //
    int update(int64_t index, int32_t val) {
      int r;
      int64_t orig_s, orig_e, orig_val,
              merge_s, merge_e;
      value_range_t *vr, *orig_vr,
                    *vr_new,
                    *vr_l, *vr_r,
                    *_vr;
      ravl_node_t *node,
                  *node_l, *node_r,
                  *node_new, *node_orig;

      if (m_tree.m_root == NULL) { return -1; }

      // find the interval where index lands
      //
      node = m_tree.m_root;
      while (node) {

        vr = (value_range_t *)(node->data);
        if (vr->e > index) {
          if (vr->s <= index) { break; }
          node = node->l;
          continue;
        }

        if (vr->s <= index) {
          if (vr->e > index) { break; }
          node = node->r;
          continue;
        }

        return -1;
      }

      // no such interval, error
      //
      if (!node) { return -1; }

      node_orig = node;
      orig_vr = (value_range_t *)(node_orig->data);

      // interval with value already exists,
      // leave it be and return soft error
      //
      if (vr->v == val) { return 1; }

      // orig_vr might disapper below, save the values
      //
      orig_s    = orig_vr->s;
      orig_e    = orig_vr->e;
      orig_val  = orig_vr->v;

      // create our new tiny interval with the new value
      //
      vr_new = _vr_create(val, index, index+1);

      // if the interval in question has span less
      // than our start, we're going to split the node,
      // assigning the original start to the newly created
      // node.
      //
      vr_l = NULL;
      if (orig_s < index) {
        vr_l = _vr_create(orig_val, orig_s, index);
      }

      vr_r = NULL;
      if (orig_e > (index+1)) {
        vr_r = _vr_create(orig_val, index+1, orig_e);
      }

      r = m_tree.del(orig_vr);
      if (r<0) { return -1; }

      orig_vr = NULL;
      m_node_count--;

      // create left and right new intervals,
      // as necessary
      //
      if (vr_l) {
        r = m_tree.add(vr_l);
        if (r<0) { return -1; }
        m_node_count++;
      }
      if (vr_r) {
        r = m_tree.add(vr_r);
        if (r<0) { return -1; }
        m_node_count++;
      }
      node = m_tree.add_p(vr_new);
      m_node_count++;

      merge_s = index;
      merge_e = index+1;

      // merge left and right intervals,
      // making sure not to use freed data
      //
      node_l = m_tree.pred(node);
      if (node_l) {
        vr_l = (value_range_t *)(node_l->data);

        if (vr_l->v == val) {
          merge_s = vr_l->s;
          m_tree.del(node_l->data);

          vr_new->s = merge_s;
          m_node_count--;
        }
      }

      node_r = m_tree.succ(node);
      if (node_r) {
        vr_r = (value_range_t *)(node_r->data);

        if (vr_r->v == val) {
          merge_e = vr_r->e;
          m_tree.del(node_r->data);

          vr_new->e = merge_e;
          m_node_count--;
        }
      }

      return 0;
    }

    RAVL m_tree;
    int64_t m_node_count;
    int64_t m_start;
    int64_t m_length;
};

//------------------
//------------------
//------------------


typedef struct cumulative_interval_range_type {
  int32_t count_l, count_r;
  int32_t s, e;
} cir_t;

static cir_t *_cir_create(int32_t s, int32_t e) {
  cir_t *cir;
  cir = (cir_t *)malloc(sizeof(cir_t));
  cir->count_l = 0;
  cir->count_r = 0;
  cir->s = s;
  cir->e = e;
  return cir;
}

static int _cir_cmp(void *_a, void *_b) {
  cir_t *a, *b;

  a = (cir_t *)_a;
  b = (cir_t *)_b;

  if (a->s < b->s) { return -1; }
  if (a->s > b->s) { return  1; }

  return 0;
}

static void _cir_free(void *_a) { free(_a); }

static void _cir_print(void *_a) {
  cir_t *a;
  a = (cir_t *)_a;
  printf("[%i:%i]{l:%i,r:%i}\n",
      (int)a->s, (int)a->e,
      (int)a->count_l, (int)a->count_r);
}

static void _cir_node_print(ravl_node_t *node) {
  _cir_print(node->data);
}

static void _cir_update(ravl_node_t *node) {
  cir_t *cir=NULL,
        *child_cir=NULL;

  if (!node) { return; }
  cir = (cir_t *)node->data;

  if (node->l) {
    child_cir = (cir_t *)(node->l->data);
    cir->count_l  = (child_cir->count_l + child_cir->count_r);
    cir->count_l += (child_cir->e - child_cir->s);
  }
  else { cir->count_l = 0; }

  if (node->r) {
    child_cir = (cir_t *)(node->r->data);
    cir->count_r  = (child_cir->count_l + child_cir->count_r);
    cir->count_r += (child_cir->e - child_cir->s);
  }
  else { cir->count_r = 0; }

  return;
}




// run length encoded tree, sparse, linear biased
//
// The idea is to allow for a list of elements from [0..n)
// (0 to (n-1) inclusive) that can be accessed, removed
// or added to efficiently.
// In addition, we want an 'index' operation, allowing us
// to access the k'th element efficiently.
//
// The idea is to store intervals that represent a contiguous
// increasing sequence of elements.
// Each of these intervals is stored in a balanced tree node
// allowing for efficient inclusivity tests (O(lg(n) on
// start value).
//
// In addition, a count of how many elements are in each of the
// children, including the interval range, is kept, allowing
// us to do index lookups.
//
// The main workflow is:
//
// T.init(s,e)
// ...
// T.rem(val)
// ...
// T.add(val)
// T.exists(val)
// T.read(&val, idx);
// ...
// T.destroy()
//
//
class RLET_SLB {
  public:


    RLET_SLB() {
      m_tree.m_update     = _cir_update;
      m_tree.m_cmp        = _cir_cmp;
      m_tree.m_free       = _cir_free;
      m_tree.m_node_print = _cir_node_print;

      m_node_count = 0;
      m_start = 0;
      m_length = 0;
    }

    ~RLET_SLB() {
      m_tree.destroy();
    }

    void destroy() { m_tree.destroy(); }

    void print() {
      printf("m_tree: node_count:%i, start:%i, length:%i\n",
          (int)m_node_count, (int)m_start, (int)m_length);
      m_tree.print_tree();
    }

    // initialize structre.
    // Create initial node which holds the range
    //
    int init(int32_t s, int32_t e) {
      int r;
      cir_t *cir;

      m_tree.destroy();
      if (e<=s) { return 0; }

      m_length = e - s;
      m_node_count=1;
      cir = _cir_create(s,e);
      r = m_tree.add(cir);
      if (r<0) { return r; }

      return 0;
    }

    int32_t count() {
      cir_t *cir;
      if (!m_tree.m_root) { return 0; }
      cir = (cir_t *)(m_tree.m_root->data);
      return (cir->count_l + cir->count_r + (cir->e - cir->s));
    }

    // check intervals don't overlap
    // check interval/index counts match up.
    // recursively go through.
    // Thsi can be slow as it might make multiple
    // sweeps through the tree
    //
    // return:
    //
    //  0 : consistent
    // <0 : inconsistent
    //
    int consistency_r(ravl_node_t *node, int lvl=0) {
      int r;
      int32_t s;
      cir_t *cir, *cir_l, *cir_r;
      ravl_node_t *node_l, *node_r;

      if (!node) { return 0; }

      cir = (cir_t *)(node->data);
      if (!cir) { return -1; }

      if (cir->e <= cir->s) { return -2; }

      node_l = m_tree.pred(node);
      if (node_l) {
        cir_l = (cir_t *)(node_l->data);
        if (cir_l->s >= cir->s) { return -3; }
        if (cir_l->e >= cir->s) { return -5; }
      }

      node_r = m_tree.succ(node);
      if (node_r) {
        cir_r = (cir_t *)(node_r->data);
        if (cir_r->s <= cir->e) { return -4; }
        if (cir_r->e <= cir->e) { return -6; }
      }

      if (node->l) {
        node_l = node->l;
        cir_l = (cir_t *)(node_l->data);
        s = cir_l->count_l + cir_l->count_r + (cir_l->e - cir_l->s);
        if (s != cir->count_l) { return -7; }
      }

      if (node->r) {
        node_r = node->r;
        cir_r = (cir_t *)(node_r->data);
        s = cir_r->count_l + cir_r->count_r + (cir_r->e - cir_r->s);
        if (s != cir->count_r) { return -8; }
      }

      r = consistency_r(node->l, lvl+1);
      if (r<0) { return r; }

      r = consistency_r(node->r, lvl+1);
      if (r<0) { return r; }

      return 0;
    }

    // check consistency of nodes.
    //
    // return:
    //
    //  0 : consistent
    // <0 : inconsistent
    //
    int consistency() {
      int r;
      int32_t s;
      cir_t *cir;
      ravl_node_t *node;

      if (!m_tree.m_root) {
        if (m_node_count != 0) { return -1; }
        return 0;
      }

      node = m_tree.m_root;
      cir = (cir_t *)node->data;
      s = cir->count_l + cir->count_r + (cir->e - cir->s);

      //if (s != m_length) { return -2; }
      r = consistency_r(node, 0);
      return r;
    }

    // Find the value at index
    //
    // return:
    //
    //  0 : success, val, if non-null, populated with value
    // <0 : error or not found
    //
    int read(int32_t *val, int32_t index) {
      int32_t idx_s = 0,
              prv_s = 0,
              cur_s = 0,
              d_idx = 0;
      ravl_node_t *node, *node_nxt;
      cir_t *cir;

      node_nxt = m_tree.m_root;
      while (node_nxt) {
        node = node_nxt;
        cir = (cir_t *)(node->data);
        cur_s = prv_s + cir->count_l;

        if (index < cur_s) {
          node_nxt = node->l;
          continue;
        }

        d_idx = cir->e - cir->s;

        if (index >= (cur_s + d_idx)) {
          prv_s = cur_s + d_idx;
          node_nxt = node->r;
          continue;
        }

        if (val) { *val = cir->s + (index - cur_s); }
        return 0;
      }

      return -1;
    }

    // get index from val
    //
    // node holds cir_t data
    // cir_t data holds:
    //   - sum of interval lengths in count_l
    //   - sum of interval lengths in count_r
    //   - start and end (non-inclusive) of interval
    //
    // like the `exists` function below, we can traverse
    // nodes, adding to our current start if we turn right
    // from the left child and node interval.
    //
    // Return:
    //
    //  0 : success, if _index is non-null, *_index holds index where val was found
    // <0 : error (value not found)
    //
    int index(int32_t *_index, int32_t val) {
      int32_t idx_s = 0,
              prv_s = 0,
              cur_s = 0,
              d_idx = 0;
      ravl_node_t *node;
      cir_t *cir;

      node = m_tree.m_root;
      while (node) {
        cir = (cir_t *)(node->data);
        cur_s = prv_s + cir->count_l;

        if (val < cir->s) {
          node = node->l;
          continue;
        }

        d_idx = cir->e - cir->s;

        if (val >= cir->e) {
          node = node->r;

          prv_s = cur_s + d_idx;
          continue;
        }

        if ((cir->s <= val) &&
            (val < cir->e)) {
          if (_index) { *_index = cur_s + (val - cir->s); }
          return 0;
        }
        return -1;
      }
      return -1;
    }

    // return:
    //
    //  1  : val exists in structure
    //  0  : val does not exist in structure
    //
    int exists(int32_t val) {
      ravl_node_t *node;
      cir_t *cir;

      node = m_tree.m_root;
      while (node) {
        cir = (cir_t *)(node->data);
        if (val < cir->s) {
          node = node->l;
          continue;
        }
        if (val >= cir->e) {
          node = node->r;
          continue;
        }

        if ((cir->s <= val) &&
            (val < cir->e)) {
          return 1;
        }
        return 0;
      }
      return 0;
    }

    // find node that houses value.
    //
    // return:
    //
    //   ravl_node_t * pointer to node on success
    //   NULL on not found or error
    //
    ravl_node_t *find_node(int32_t val) {
      ravl_node_t *node;
      cir_t *cir;

      node = m_tree.m_root;
      while (node) {
        cir = (cir_t *)(node->data);
        if (val < cir->s) {
          node = node->l;
          continue;
        }
        if (val >= cir->e) {
          node = node->r;
          continue;
        }

        if ((cir->s <= val) &&
            (val < cir->e)) {
          return node;
        }
        return NULL;
      }
      return NULL;
    }

    // recursively update node and its parents.
    // Used to keep track of the left and right
    // count.
    // Under tree insertion/deleteions it should
    // recursively go through and call update
    // to all parent nodes affetcted but sometimes
    // we want to update the nodes explicitely
    // ourselves, without doing an balance tree
    // insert or delete operation.
    //
    int update_r(ravl_node_t *node) {
      ravl_node_t *nl, *nr;
      cir_t *cir, *cl, *cr;
      int32_t count;

      if (!node) { return 0; }

      cir = (cir_t *)(node->data);

      if (node->l) {
        nl = node->l;
        cl = (cir_t *)(nl->data);
        cir->count_l = cl->count_l + cl->count_r + (cl->e - cl->s);
      }
      else {
        cir->count_l = 0;
      }

      if (node->r) {
        nr = node->r;
        cr = (cir_t *)(nr->data);
        cir->count_r = cr->count_l + cr->count_r + (cr->e - cr->s);
      }
      else {
        cir->count_r = 0;
      }

      return update_r(node->p);
    }


    // add val.
    //
    // Internally, this will merge contiguous
    // regions neighboring any newly created
    // node.
    //
    // Return:
    //
    //  0 : success
    //  1 : element already exists, nothing done
    // <0 : error
    //
    int add(int32_t val) {
      ravl_node_t *node,
                  *tnode;
      cir_t *cir, *tcir;
      int32_t merge_s, merge_e;

      // if it already exists, we can
      // just do nothing other than
      // return a soft error code
      //
      node = find_node(val);
      if (node) { return 1; }

      cir = _cir_create(val,val+1);
      node = m_tree.add_p(cir);

      tnode = m_tree.succ(node);
      if (tnode) {
        tcir = (cir_t *)(tnode->data);
        if (tcir->s == cir->e) {
          merge_e = tcir->e;
          m_tree.del(tcir);

          cir->e = merge_e;
          update_r(node);
        }
      }

      tnode = m_tree.pred(node);
      if (tnode) {
        tcir = (cir_t *)(tnode->data);
        if (tcir->e == cir->s) {
          merge_s = tcir->s;
          m_tree.del(tcir);

          cir->s = merge_s;
          update_r(node);
        }
      }

      m_length++;

      return 0;
    }

    // return:
    //
    //  0 : success
    // -1 : error (not found)
    //
    int rem(int32_t val) {
      int r;
      ravl_node_t *node;
      cir_t *cir,
            *cir_l,
            *cir_r,
            orig;

      node = find_node(val);
      if (!node) { return 1; }

      cir = (cir_t *)(node->data);

      orig.s = cir->s;
      orig.e = cir->e;
      orig.count_l = cir->count_l;
      orig.count_r = cir->count_r;

      r = m_tree.del(&orig);
      if (r<0) { return r; }

      cir_l = NULL;
      if (orig.s < val) {
        cir_l = _cir_create(orig.s, val);
        r = m_tree.add(cir_l);
        if (r<0) { return r; }
      }

      cir_r = NULL;
      if (orig.e > (val+1)) {
        cir_r = _cir_create(val+1, orig.e);
        r = m_tree.add(cir_r);
        if (r<0) { return r; }
      }

      m_length--;

      return 0;
    }

    // 
    int copy_fast_r(ravl_node_type *dst_node, ravl_node_type *src_node) {
      int r;
      cir_t *src_data, *dst_data;
      ravl_node_type *child_node;

      if ((!dst_node) && (!src_node)) { return 0; }
      if (!dst_node) { return -1; }
      if (!src_node) { return -1; }

      if (src_node->l) {
        src_data = (cir_t *)(src_node->l->data);
        dst_data = _cir_create(src_data->s, src_data->e);

        dst_data->count_l = src_data->count_l;
        dst_data->count_r = src_data->count_r;

        child_node = (ravl_node_t *)malloc(sizeof(ravl_node_t));
        child_node->p = dst_node;
        child_node->l = NULL;
        child_node->r = NULL;
        child_node->dh = src_node->l->dh;
        child_node->data = dst_data;

        dst_node->l = child_node;

        r = copy_fast_r(dst_node->l, src_node->l);
        if (r<0) { return r; }
      }

      if (src_node->r) {
        src_data = (cir_t *)(src_node->r->data);
        dst_data = _cir_create(src_data->s, src_data->e);

        dst_data->count_l = src_data->count_l;
        dst_data->count_r = src_data->count_r;

        child_node = (ravl_node_t *)malloc(sizeof(ravl_node_t));
        child_node->p = dst_node;
        child_node->l = NULL;
        child_node->r = NULL;
        child_node->dh = src_node->r->dh;
        child_node->data = dst_data;

        dst_node->r = child_node;

        r = copy_fast_r(dst_node->r, src_node->r);
        if (r<0) { return r; }
      }

      return 0;
    }

    // trying to speed copy up.
    // I hate doing it because it requires intimate
    // knowledge of the underly avl node type,
    // specifically the `dh` parameter,
    // but the O(lg N) factor is too much
    // not at least experiment with
    //
    int copy_fast(RLET_SLB *src) {
      int r;
      cir_t *src_data, *dst_data;

      m_tree.destroy();

      m_node_count  = src->m_node_count;
      m_start       = src->m_start;
      m_length      = src->m_length;

      if (src->m_tree.m_root) {
        src_data = (cir_t *)(src->m_tree.m_root->data);
        dst_data = _cir_create(src_data->s, src_data->e);

        dst_data->count_l = src_data->count_l;
        dst_data->count_r = src_data->count_r;

        m_tree.m_root = (ravl_node_t *)malloc(sizeof(ravl_node_t));
        m_tree.m_root->p = NULL;
        m_tree.m_root->l = NULL;
        m_tree.m_root->r = NULL;
        m_tree.m_root->dh = src->m_tree.m_root->dh;
        m_tree.m_root->data = dst_data;
      }

      r = copy_fast_r(m_tree.m_root, src->m_tree.m_root);
      if (r<0) { return r; }

      return 0;
    }

    //int copy(RLET_SLB *src) { return copy_fast(src); }

    int copy_r(ravl_node_type *src_node) {
      int r;
      cir_t *src_data, *dst_data;

      if (!src_node) { return 0; }

      dst_data = NULL;
      if (src_node->l) {
        src_data = (cir_t *)(src_node->l->data);
        dst_data = _cir_create(src_data->s, src_data->e);

        r = m_tree.add(dst_data);
        if (r<0) { return r; }
      }

      dst_data = NULL;
      if (src_node->r) {
        src_data = (cir_t *)(src_node->r->data);
        dst_data = _cir_create(src_data->s, src_data->e);

        r = m_tree.add(dst_data);
        if (r<0) { return r; }
      }

      r = copy_r(src_node->l);
      if (r<0) { return r; }

      r = copy_r(src_node->r);
      if (r<0) { return r; }

      return 0;
    }

    // functional copy
    //
    int copy(RLET_SLB *src) {
      int r;
      cir_t *src_data, *dst_data;

      m_tree.destroy();

      m_node_count  = src->m_node_count;
      m_start       = src->m_start;
      m_length      = src->m_length;

      if (src->m_tree.m_root) {
        src_data = (cir_t *)(src->m_tree.m_root->data);
        dst_data = _cir_create(src_data->s, src_data->e);

        r = m_tree.add(dst_data);
        if (r<0) { return r; }
      }

      r = copy_r(src->m_tree.m_root);
      if (r<0) { return r; }

      return 0;
    }

    RAVL m_tree;
    int32_t m_node_count;
    int32_t m_start;
    int32_t m_length;
};

#endif
