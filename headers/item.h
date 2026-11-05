// !!!! ITEM DUMMY TEMPORARIO !!!! //

#ifndef ITEM_H
#define ITEM_H

typedef struct item_ ITEM;

ITEM* item_criar();
ITEM* item_copiar(ITEM*); //retorna um item de conteúdo igual
void item_apagar(ITEM**);

int item_get_id(ITEM*);

#endif /* ifndef ITEM_H */
