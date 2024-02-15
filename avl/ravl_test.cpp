//
// To the extent possible under law, the person who associated CC0 with
// this file has waived all copyright and related or neighboring rights
// to this file.
//
// You should have received a copy of the CC0 legalcode along with this
// work.  If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>

#include "ravl.hpp"

typedef struct data64_type {
  int64_t key;
  int64_t value;
} data64_t;

data64_t *data64_create(int64_t key, int64_t val) {
  data64_t *d;
  d = (data64_t *)malloc(sizeof(data64_t));
  d->key = key;
  d->value = val;
  return d;
}

int cmp_data64(void *_a, void *_b) {
  data64_t *a = (data64_t *)_a,
           *b = (data64_t *)_b;
  if (a->key < b->key) { return -1; }
  if (a->key > b->key) { return  1; }
  return 0;
}

void free_data64(void *_a) { free(_a); }

void update_data64(ravl_node_t *node) {

}

void __node_print(ravl_node_t *node) {
  data64_t *d;
  d = (data64_t *)node->data;
  printf("#_node_print: k:%i, v:%i\n",
      (int)d->key, (int)d->value);
}

void _node_print(ravl_node_t *node) {
  data64_t *d;
  d = (data64_t *)node->data;

  printf("{%p} dh:%i, k:%i, v:%i\n",
      node,
      node->dh,
      (int)d->key, (int)d->value);

  /*
  printf("(%s){%p} dh:%i, k:%i, v:%i\n",
      node->name.c_str(), node,
      node->dh,
      (int)d->key, (int)d->value);
      */
}

int test_ins() {
  int i;
  data64_t *dat;
  RAVL tree;

  int n_test = 100;

  tree.m_cmp = cmp_data64;
  tree.m_free = free_data64;
  tree.m_update = update_data64;
  tree.m_node_print = _node_print;

  printf("... %i\n", tree.consistency_check());

  for (i=0; i<n_test; i++) {

    printf("-----\n");

    dat = data64_create( rand()%20, rand()%30 );
    tree.add(dat);

    printf("[%i] consistency:%i\n", i, tree.consistency_check());
    //tree.print_r(tree.m_root, 0);

    printf("\n=============tree================\n");
    tree.print_tree();
    printf("=============tree================\n\n");

    printf("-----\n\n");
  }


  printf("... %i\n", tree.consistency_check());

  tree.destroy();
  return 0;
}

int test_add_del() {
  int i, r;
  data64_t *dat,
           query;
  RAVL tree;

  int n_test = 1000;

  tree.m_cmp = cmp_data64;
  tree.m_free = free_data64;
  tree.m_update = update_data64;
  tree.m_node_print = _node_print;

  printf("... %i\n", tree.consistency_check());

  for (i=0; i<n_test; i++) {

    printf("-----\n");

    if (rand()%2) {
      printf("ADD\n");
      dat = data64_create( rand()%20, rand()%30 );
      tree.add(dat);
    }
    else {
      query.key = rand()%20;
      query.value = rand()%30;

      printf("DEL {k:%i,v:%i}\n", (int)query.key, (int)query.value);

      r = tree.del(&query);
      printf("# del, attempted to del {k:%i,v:%i} got %i\n",
          (int)query.key, (int)query.value, r);
    }

    printf("[%i] consistency:%i\n", i, tree.consistency_check());
    //tree.print_r(tree.m_root, 0);

    printf("\n=============tree================\n");
    tree.print_tree();
    printf("=============tree================\n\n");

    printf("-----\n\n");
  }


  printf("... %i\n", tree.consistency_check());

  tree.destroy();
  return 0;

}

int main(int argc, char **argv) {

  //test_ins();
  test_add_del();
}

