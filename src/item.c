// !!!! ITEM DUMMY TEMPORARIO !!!! //

#include <stdlib.h>
#include <time.h>

#include "../headers/item.h"

struct item_ {
  int id;
};

ITEM* item_criar() {
  srand(time(NULL));

  ITEM *item = (ITEM*) malloc(sizeof(ITEM));
  if (item != NULL) item->id = rand() % 10000;

  return item;
}

ITEM* item_copiar(ITEM *item) {
  if (item == NULL) return NULL;

  ITEM *res = (ITEM*) malloc(sizeof(ITEM));
  if (res == NULL) return NULL;

  res->id = item->id;
  return res;
}

void item_apagar(ITEM **item) {
  if (item == NULL || *item == NULL) return;

  free(*item);
  *item == NULL;
}

int item_get_id(ITEM *item) {
  return item != NULL ? item->id : -1;
}
