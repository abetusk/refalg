//
// To the extent possible under law, the person who associated CC0 with
// this file has waived all copyright and related or neighboring rights
// to this file.
//
// You should have received a copy of the CC0 legalcode along with this
// work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//

#ifndef RAVL_H
#define RAVL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct ravl_node_type {

  // balance factor (delta height)
  // this is:
  //
  //   height(rchild) - height(lchild)
  //
  int8_t dh;

  // p - parent
  // l - left child
  // r - right child
  //
  struct ravl_node_type *p,
                        *l,
                        *r;

  // used as key and arbitrary data
  //
  void *data;

} ravl_node_t;

class RAVL {
  public:

    RAVL() {
      m_cmp = NULL;
      m_free = NULL;
      m_update = NULL;
      m_node_print = NULL;

      m_root = NULL;

      m_node_count = 0;
      m_depth = 0;
    }

    ~RAVL() {
      destroy();
    }

    // add_p returns pointer to newly created
    // ravl_node_t.
    //
    int add(void *);
    ravl_node_t *add_p(void *);

    // query data is left untouched and
    // is only used to find entry.
    //
    int del(void *);

    int destroy();
    int destroy_r(ravl_node_t *);

    ravl_node_t *search(void *);

    int retrace(ravl_node_t *, int8_t);

    //----

    ravl_node_t *succ(ravl_node_t *);
    ravl_node_t *pred(ravl_node_t *);

    //----

    void print_tree_r(ravl_node_t *, int);
    void print_tree();

    int print_r(ravl_node_t *node, int lvl);
    int check_cmp_r(ravl_node_t *node);
    int check_height_r(ravl_node_t *);
    int consistency_check(void);

    //----

    ravl_node_t *m_root;

    int64_t m_node_count;
    int64_t m_depth;

    int  (*m_cmp)(void *a, void *b);
    void (*m_free)(void *a);
    void (*m_update)(ravl_node_t *);

    void (*m_node_print)(ravl_node_t *);
};

#endif
