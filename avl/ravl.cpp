//
// To the extent possible under law, the person who associated CC0 with
// this file has waived all copyright and related or neighboring rights
// to this file.
//
// You should have received a copy of the CC0 legalcode along with this
// work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//

#include "ravl.hpp"

#define RAVL_C_MEM_ALLOC

//#include "dulux.h"

// user is in charge of memory management for
// data.
// m_free for data
// will be called when this node is deleted
// User is expected to allocate data before
// adding or calling this function, this will
// only assign the ravl_node_t it's data.
//
ravl_node_t *ravl_node_create(ravl_node_t *parent,
                         void *data) {
  static int idx=0;
  ravl_node_t *node;

#ifdef RAVL_C_MEM_ALLOC
  node = (ravl_node_t *)malloc(sizeof(ravl_node_t));
#else
  node = new ravl_node_t ;
#endif

  node->dh = 0;
  //node->depth = 0;

  node->p = parent;
  node->l = NULL;
  node->r = NULL;

  node->data = data;

  //idx = rand() % str_dulux_n[0];
  //node->name = str_dulux[idx];

  return node;
}

void ravl_node_free(ravl_node_t *node) {
#ifdef RAVL_C_MEM_ALLOC
  free(node);
#else
  delete node;
#endif
}

void RAVL::print_tree_r(ravl_node_t *node, int lvl) {
  int i;

  if (!node) { return; }

  print_tree_r(node->r, lvl+1);

  for (i=0; i<lvl; i++) { printf(" "); }
  if (m_node_print) { m_node_print(node); }

  print_tree_r(node->l, lvl+1);
}

void RAVL::print_tree() {
  if (!m_root) { return; }
  print_tree_r(m_root, 0);
}

int RAVL::print_r(ravl_node_t *node, int lvl) {
  int i;
  if (!node) { return 0; }

  if (m_node_print) { m_node_print(node); }

  for (i=0; i<lvl; i++) { printf(" "); }
  printf("%p dh:%i, {p:%p,l:%p,r:%p}\n", node, node->dh, node->p, node->l, node->r);

  print_r(node->l, lvl+1);
  print_r(node->r, lvl+1);
  return 0;
}

//int RAVL::init() { return 0; }

int RAVL::check_height_r(ravl_node_t *node) {
  int h_l, h_r;
  if (!node) { return 0; }

  h_l = check_height_r(node->l);
  h_r = check_height_r(node->r);

  if ((h_l<0) || (h_r<0)) { return -1; }
  if ((h_r - h_l) != node->dh) { return -1; }
  if (abs(h_r-h_l) > 1) { return -1; }

  if (h_r > h_l) { return h_r + 1; }
  return h_l+1;
}

int RAVL::check_cmp_r(ravl_node_t *node) {
  int r;
  ravl_node_t *node_left,
              *node_right;
  void *dat_root, *dat_child;

  if (!m_cmp) { return -1; }
  if (!node) { return 0; }
  node_left  = node->l;
  node_right = node->r;

  if (node_left) {
    dat_root = node->data;
    dat_child = node_left->data;

    printf("l_cmp:\n");
    if (m_node_print) { m_node_print(node_left); }
    if (m_node_print) { m_node_print(node); }

    r = m_cmp(dat_child, dat_root);
    printf("m_cmp(child,root):%i\n", r);

    if (m_cmp(dat_child, dat_root) > 0) { return -2; }

    r = check_cmp_r(node_left);
    if (r<0) { return r; }
  }

  if (node_right) {
    dat_root = node->data;
    dat_child = node_right->data;

    printf("r_cmp:\n");
    if (m_node_print) { m_node_print(node); }
    if (m_node_print) { m_node_print(node_right); }

    r = m_cmp(dat_root, dat_child);
    printf("m_cmp(root,child):%i\n", r);


    if (m_cmp(dat_root, dat_child) > 0) { return -3; }

    r = check_cmp_r(node_right);
    if (r<0) { return r; }
  }

  return 0;
}

int RAVL::consistency_check() {
  int h_l, h_r, r;
  if (!m_cmp) { return -1; }
  if (!m_root) { return 0; }
  if (m_root->p != NULL) { return -4; }

  h_l = check_height_r( m_root->l );
  h_r = check_height_r( m_root->r );
  if ((h_l<0) || (h_r<0)) { return -2; }
  if ((h_r-h_l) != m_root->dh) { return -3; }
  if (abs(h_r-h_l) > 1) { return -5; }

  r = check_cmp_r(m_root);
  if (r<0) { return r; }
  

  return 0;
}

//---

// recursively go up and rebalnce the tree,
// making sure to call the user specified
// calback on nodes that have finished processing
// and whose children are settled.
//
// node dh has alredy been updated by the time
// we get here.
// dT holds the change in subtree height,
// -1 for reduction in height (deletion), +1
// for addition.
//
// some comments:
//
// - dT is the comunication of the height change
//   from subtree
// - dT should be {-1,0,1}, {-1,0} for del, {0,1} for add
// - The nodes dh (balance factor) in addition to dT
//   gives us enough information to know how to
//   modify the dh (balance factor) of the parent
//   and what height change to recursively pass up
//   the tree.
//
// return:
//
//  0 : success
//
int RAVL::retrace(ravl_node_t *node, int8_t dT) {
  int r = 0;
  int32_t del_h;
  int8_t l_dh=0, r_dh=0,
         xu_dh=0, yu_dh=0, zu_dh=0,
         pu_dh=0;
  ravl_node_t *x, *y, *z, *p;
  ravl_node_t *alpha,
              *beta,
              *gamma,
              *rho;

  if (!node) { return 0; }


  // If we've deleted a node (dT < 0) and
  // dh went from +-1 to 0, we need to communicate
  // that height change up.
  //
  // Otherwise if dT >= 0 we've either done nothing
  // or an insertion, meaning the insertion, if
  // there was one, got absorbed without a height change.
  //
  //
  if (node->dh == 0) {

    p = node->p;
    if (p && (dT < 0)) {
      if (p->l == node) { p->dh++; }
      else              { p->dh--; }
    }

    if (m_update) { m_update(node); }
    return retrace(node->p, (dT < 0) ? -1 : 0);
  }

  // If the node doesn't need rebalacing,
  // but we still need to take care if
  // the height has changed.
  //
  if ((node->dh <=  1) &&
      (node->dh >= -1)) {

    if (m_update) { m_update(node); }

    // If there's been no height change, then go one.
    // If there's been a deletion (dT < 0), we must
    // have transitioned from dh = 0 -> dh = {-1,1},
    // so the height change got absorbed into this
    // node.
    //
    if (dT <= 0) { return retrace(node->p, 0); }

    // else, the height has changed and we need to update
    // the parent accordingly.
    //
    if (node->p) {
      if (node->p->l == node) { node->p->dh--; }
      else                    { node->p->dh++; }
    }

    // including sending the height change signal up the
    // chain
    //
    return retrace(node->p, dT);
  }

  // Otherwise, we need to do a rotation.
  // If we get here, dT should be either -1 or 1,
  // with the only transitions possible
  // from (- -> --) or (+ -> ++) 
  //

  if (node->l) { l_dh = (int32_t)(node->l->dh); }
  if (node->r) { r_dh = (int32_t)(node->r->dh); }

  // left heavy (--)
  //
  if (node->dh < -1) {

    // doublerot (--) (+) ( x{--} ( y{+} (alpha, z (beta, gamma), rho) ) ) 
    //
    // The height change from the original (pre-insert) will not
    // change, so no height change needs to be communicated to
    // the parent.
    //
    if (l_dh == 1) {

      x = node;
      y = x->l;
      z = y->r;
      alpha = y->l;
      beta  = z->l;
      gamma = z->r;
      rho   = x->r;

      xu_dh = ((z->dh == -1) ?  1 : 0);
      yu_dh = ((z->dh ==  1) ? -1 : 0);

      p = x->p;
      if (p) {
        if (p->l == x)  {
          p->l = z;
          if (dT < 0) { p->dh++; }
        }
        else {
          p->r = z;
          if (dT < 0) { p->dh--; }
        }

      }
      if (m_root == x) { m_root = z; }

      z->p = p;

      z->l = y;      z->r = x;
      x->p = z;      y->p = z;

      y->l = alpha;  y->r = beta;
      if (alpha)  { alpha->p = y; }
      if (beta)   { beta->p  = y; }

      x->l = gamma;  x->r = rho;
      if (gamma)  { gamma->p = x; }
      if (rho)    { rho->p   = x; }

      x->dh = xu_dh;
      y->dh = yu_dh;
      z->dh = 0;

      if (m_update) {
        m_update(y);
        m_update(x);
        m_update(z);
      }

      if (dT < 0) { return retrace(z->p, -1); }
      return retrace(z->p, 0);
    }

    // simplerot -- {-,0}
    //
    // There will only be a height change to the parent of this
    // subtree in the case the left child has dh=0;
    //

    xu_dh = ((l_dh == 0) ? -1 :  0);
    yu_dh = ((l_dh == 0) ?  1 :  0);
    pu_dh = ((l_dh == 0) ?  1 :  0);

    x = node;
    y = node->l;
    beta = y->r;

    // fix up x's parent
    // and repoint root node if
    // necessary
    //
    p = x->p;
    if (p) {

      if (p->l == x)  {
        p->l = y;

        if      ((l_dh == -1) && (dT < 0)) { p->dh++; }
        else if ((l_dh ==  0) && (dT > 0)) { p->dh--; }
      }

      else {
        p->r = y;

        if      ((l_dh == -1) && (dT < 0)) { p->dh--; }
        else if ((l_dh ==  0) && (dT > 0)) { p->dh++; }
      }

    }
    if (m_root == x) { m_root = y; }

    y->p = x->p;
    y->r = x;
    x->p = y;

    x->l = beta;
    if (beta) { beta->p = x; }

    x->dh = xu_dh;
    y->dh = yu_dh;

    if (m_update) {
      m_update(x);
      m_update(y);
    }

    if (l_dh == 0) {
      return retrace(p, (dT > 0) ?  1 : 0);
    }
    return retrace(p, (dT < 0) ? -1 : 0);
  }

  // doublerot (++) (-) (node->dh > 1, node->r->dh == -1)
  // For a double rotation, the subtree will not
  // have changed its height from before the insertion,
  // so not height change or alteration needs to be communicated
  // up to the parent.
  //
  if (r_dh == -1) {

    x = node;
    y = x->r;
    z = y->l;

    alpha = x->l;
    beta  = z->l;
    gamma = z->r;
    rho   = y->r;

    xu_dh = ((z->dh ==  1) ? -1: 0);
    yu_dh = ((z->dh == -1) ?  1: 0);

    p = x->p;
    if (p) {

      if (p->l == x) {
        p->l = z;
        if (dT < 0) { p->dh++; }
      }

      else {
        p->r = z;
        if (dT < 0) { p->dh--; }
      }

    }
    if (m_root == x) { m_root = z; }

    z->p = p;

    z->l = x;       z->r = y;
    x->p = z;       y->p = z;

    x->l = alpha;   x->r = beta;
    if (alpha)  { alpha->p = x; }
    if (beta)   { beta->p  = x; }

    y->l = gamma;   y->r = rho;
    if (gamma)  { gamma->p = y; }
    if (rho)    { rho->p   = y; }

    x->dh = xu_dh;
    y->dh = yu_dh;
    z->dh = 0;

    if (m_update) {
      m_update(x);
      m_update(y);
      m_update(z);
    }

    return retrace(z->p, (dT < 0) ? -1 : 0);
  }

  // simple rotation
  // right heavy (++) {+,0}
  //
  // There will be a height change of the subtree from
  // the original only in the case when the right child's
  // dh=0.
  //

  xu_dh = ((r_dh == 0) ?  1 : 0);
  yu_dh = ((r_dh == 0) ? -1 : 0);

  x = node;
  y = node->r;
  beta = y->l;

  p = x->p;
  if (p) {
    if (p->l == x)  {
      p->l = y;

      if      ((r_dh == 1) && (dT < 0)) { p->dh++; }
      else if ((r_dh == 0) && (dT > 0)) { p->dh--; }
    }
    else {
      p->r = y;

      if      ((r_dh == 1) && (dT < 0)) { p->dh--; }
      else if ((r_dh == 0) && (dT > 0)) { p->dh++; }
    }

  }
  if (m_root == x) { m_root = y; }

  y->p = p;
  y->l = x;
  x->p = y;

  x->r = beta;
  if (beta) { beta->p = x; }

  x->dh = xu_dh;
  y->dh = yu_dh;

  if (m_update) {
    m_update(x);
    m_update(y);
  }

  if (r_dh == 0) {
    return retrace(p, (dT > 0) ? 1 : 0);
  }
  return retrace(p, (dT < 0) ? -1 : 0);
}


// Creates a new ravl_node_t (with malloc)i
// with kd data and adds it to the tree.
//
// If the tree is empty, m_root will be set to the
// newly created node.
//
// Otherwise, a binary search will occur and the
// new node will be inserted at the appropriate location.
//
// m_update, if non-null, will be called on the newly
// added node.
//
// retrace will be called on the parent.
//
// Return:
//
// 0  - success
// !0 - error
//
ravl_node_t *RAVL::add_p(void *kd) {
  int r, c;
  int8_t dT=0;
  ravl_node_t *node,
              *node_nxt,
              *nn;

  // initialize root node,
  // insert and nothing more to do
  //
  if (!m_root) {
    m_root = ravl_node_create(NULL, kd);
    m_depth++;
    m_node_count++;
    if (m_update) { m_update(m_root); }
    return m_root;
  }

  // otherwise, find leaf node to
  // insert under
  //
  node_nxt = m_root;
  while (node_nxt) {
    node = node_nxt;
    c = m_cmp(kd, node->data);
    node_nxt = ( (c<0) ? node->l : node->r );
  }

  // create a new node, using the above
  // comparitor to insert left/right
  // appropriately
  //
  // If there is a dh transition from 0->+-1,
  // we know the height of the whole subtree
  // has changed and we set dT appropriately.
  //
  nn = ravl_node_create(node, kd);
  dT = 0;
  if (c < 0)  {
    node->l = nn;
    node->dh--;

    if (node->dh < 0) { dT = 1; }

  }
  else {
    node->r = nn;
    node->dh++;

    if (node->dh > 0) { dT =  1; }

  }


  m_node_count++;
  if (m_update) {
    m_update(nn);
  }

  // handle any rebalancing that needs
  // to be done
  //
  r = retrace(node, dT);
  if (r<0) {
    ravl_node_free(nn);
    return NULL;
  }

  return nn;
}

int RAVL::add(void *kd) {
  ravl_node_t *node;
  node = add_p(kd);
  if (!node) { return -1; }
  return 0;
}

ravl_node_t *RAVL::search(void *query) {
  int r=0;
  ravl_node_t *node;
  node = m_root;
  while (node) {
    r = m_cmp(node->data, query);
    if (r==0) { return node; }
    node = ( (r<0) ? node->l : node->r );
  }
  return NULL;
}


// dkey is left untouched and is only
// used to find the appropriate entry
// to delete.
//
int RAVL::del(void *dkey) {

  int r, c;
  int8_t dT=0,
         dir_dh = 0,
         p_dh=0;
  ravl_node_t *node=NULL,
              *node_nxt=NULL,
              *nn=NULL;
  ravl_node_t *x, *p_x,
              *y, *p_y,
              *p, *node_child;
  void *tkd;

  node_nxt = m_root;
  while (node_nxt) {
    node = node_nxt;
    c = m_cmp(dkey, node->data);
    if (c==0) { break; }
    node_nxt = ( (c<0) ? node->l : node->r );
  }

  // not found or empty tree
  //
  if (!node) { return -1; }
  if (c!=0) { return -2; }

  if ((node->l) && (node->r)) {

    dT = 0;

    // succ is lower in the tree as node
    // has both children.
    //
    x = node;
    y = succ(x);

    y->dh = x->dh;

    // perform surgery to
    // take out y and put
    // it in x's place
    //
    p_y = y->p;
    if (p_y) {
      p_dh = p_y->dh;
      if (p_y->l == y)  {
        p_y->l  = y->r;
        dT      = -1;
        dir_dh  =  1;
      }
      else {
        p_y->r  = y->r;
        dT      = -1;
        dir_dh  = -1;
      }
      if (y->r) {
        y->r->p = p_y;
      }
    }

    // surgery to remove x
    //
    p_x = x->p;
    if (p_x) {
      if (p_x->l == x)  { p_x->l = y; }
      else              { p_x->r = y; }
    }
    y->p = p_x;
    y->r = x->r;
    y->l = x->l;

    if (y->r) { y->r->p = y; }
    if (y->l) { y->l->p = y; }

    if (m_root == x) { m_root = y; }

    if (p_y == x) { p_y = y; }
    if (p_y) {
      p_y->dh = p_dh + dir_dh;
    }

    if (m_free) { m_free(x->data); }
    ravl_node_free(x);

    return retrace(p_y, dT);
  }

  // one or both of the node's children
  // is null
  //

  p = node->p;
  node_child = NULL;
  if      (node->l) { node_child = node->l; }
  else if (node->r) { node_child = node->r; }

  if (p) {
    if (p->l == node) {
      p->l    = node_child;
      dT      = -1;
      dir_dh  =  1;
    }
    else {
      p->r    = node_child;
      dT      = -1;
      dir_dh  = -1;
    }
    p->dh += dir_dh;
  }
  if (node_child) { node_child->p = p; }
  if (m_root == node) { m_root = node_child; }

  if (m_free) {
    m_free(node->data);
  }
  ravl_node_free(node);

  return retrace(p, dT);
}

// predecessor
//
ravl_node_t *RAVL::pred(ravl_node_t *node) {
  ravl_node_t *t=NULL;
  if (!node) { return node; }

  t = node->l;
  if (t) {
    while (t->r) { t = t->r; }
    return t;
  }

  t = node->p;
  while (t && (node == t->l)) {
    node = t;
    t = t->p;
  }
  return t;
}

// successor
//
ravl_node_t *RAVL::succ(ravl_node_t *node) {
  ravl_node_t *t=NULL;
  if (!node) { return node; }

  t = node->r;
  if (t) {
    while (t->l) { t = t->l; }
    return t;
  }

  t = node->p;
  while (t && (node == t->r)) {
    node = t;
    t = t->p;
  }
  return t;
}

int RAVL::destroy_r(ravl_node_t *node) {
  int r;
  if (!node) { return 0; }

  r = destroy_r(node->l);
  if (r<0) { return r; }

  r = destroy_r(node->r);
  if (r<0) { return r; }

  if (m_free) {
    m_free(node->data);
  }
  ravl_node_free(node);

  return 0;
}

int RAVL::destroy(void) {
  int r;
  r = destroy_r(m_root);
  if (r<0) { return r; }

  m_root = NULL;
  m_node_count = 0;
  m_depth = 0;

  return 0;
}

