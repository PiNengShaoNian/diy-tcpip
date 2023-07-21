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

typedef struct _nlist_t {
  nlist_node_t *first;
  nlist_node_t *last;
  int count;
} nlist_t;

void nlist_init(nlist_t *list);
void nlist_insert_first(nlist_t *list, nlist_node_t *node);

static inline int nlist_count(nlist_t *list) { return list->count; }
static inline int nlist_is_empty(nlist_t *list) { return list->count == 0; }
static inline nlist_node_t *nlist_first(nlist_t *list) { return list->first; }
static inline nlist_node_t *nlist_last(nlist_t *list) { return list->last; }

#endif