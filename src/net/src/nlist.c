#include "nlist.h"

void nlist_init(nlist_t *list) {
  list->first = list->last = (nlist_node_t *)0;
  list->count = 0;
}

void nlist_insert_first(nlist_t *list, nlist_node_t *node) {
  node->pre = (nlist_node_t *)0;
  node->next = list->first;

  if (nlist_is_empty(list)) {
    list->last = list->first = node;
  } else {
    list->first->pre = node;
    list->first = node;
  }
  list->count++;
}

nlist_node_t *nlist_remove(nlist_t *list, nlist_node_t *node) {
  if (node == list->first) {
    list->first = node->next;
  }

  if (node == list->last) {
    list->last = node->pre;
  }

  if (node->pre) {
    node->pre = node->next;
  }

  if (node->next) {
    node->next->pre = node->pre;
  }

  node->pre = node->next = (nlist_node_t *)0;
  list->count--;
  return node;
}