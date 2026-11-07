#ifndef LISTA_CRUZ_H
#define LISTA_CRUZ_H

#include <stdbool.h>
#include "item.h"

typedef struct lista_cruz_ LISTA_CRUZ;

LISTA_CRUZ* lista_cruz_criar();
void lista_cruz_apagar(LISTA_CRUZ**);

bool lista_cruz_inserir(LISTA_CRUZ*, ITEM*, TEMPLATE*, char* lin_campo, char* col_campo);

bool lista_cruz_remover(LISTA_CRUZ*, int lin, int col);

ITEM* lista_cruz_get(LISTA_CRUZ*, int lin, int col);

#endif // !LISTA_CRUZ_H
