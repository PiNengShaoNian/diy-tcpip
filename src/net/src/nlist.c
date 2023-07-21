#include "nlist.h"

void nlist_init(nlist_t *list) {
  list->first = list->last = (nlist_node_t *)0;
  list->count = 0;
}