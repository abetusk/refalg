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

value_range_t *_vr_create(int32_t v, int64_t s, int64_t e) {
  value_range_t *vr;
  vr = (value_range_t *)malloc(sizeof(value_range_t));
  vr->v = v;
  vr->s = s;
  vr->e = e;
  return vr;
}

int _vr_cmp(void *_a, void *_b) {
  value_range_t *a, *b;

  a = (value_range_t *)_a;
  b = (value_range_t *)_b;

  if (a->s < b->s) { return -1; }
  if (a->s > b->s) { return  1; }

  return 0;
}

void _vr_free(void *_a) { free(_a); }

void _vr_print(void *_a) {
  value_range_t *a;
  a = (value_range_t *)_a;
  printf("[%i:%i]{v:%i}\n",
      (int)a->s, (int)a->e, (int)a->v);
}

void _node_print(ravl_node_t *node) {
  _vr_print(node->data);
}

void _vr_update(ravl_node_t *node) {
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

      vr = (value_range_t *)malloc(sizeof(value_range_t));
      vr->v = val;
      vr->s = s;
      vr->e = e_ninc;

      return m_tree.add(vr);
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
      return consistency_r(m_tree.m_root, 0);
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

#endif
