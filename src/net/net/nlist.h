#ifndef NLIST_H
#define NLIST_H

typedef struct _nlist_node_t {
  struct _nlist_node_t *pre;
  struct _nlist_node_t *next;
} nlist_node_t;

static inline void nlist_node_init(nlist_node_t *node) {
  node->next = node->pre = (nlist_node_t *)0;
}

static inline nlist_node_t *nlist_node_next(nlist_node_t *node) {
  return node->next;
}

static inline nlist_node_t *nlist_node_pre(nlist_node_t *node) {
  return node->pre;
}

static inline void nlist_node_set_next(nlist_node_t *node, nlist_node_t *next) {
  node->next = next;
}

#endif