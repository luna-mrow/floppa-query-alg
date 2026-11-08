#ifndef LISTA_CRUZ_H
#define LISTA_CRUZ_H

#include <stdbool.h>
#include "lista.h"

typedef struct lista_cruz_ LISTA_CRUZ;

LISTA_CRUZ* lista_cruz_criar();
void lista_cruz_apagar(LISTA_CRUZ**);

bool lista_cruz_inserir(LISTA_CRUZ*, ITEM*, const char *lin_campo, const char *col_campo);

//uma desvantagem é que ao invés de remover um item específico, remove todos os itens em uma certa posição
bool lista_cruz_remover(LISTA_CRUZ*, int lin, int col);

LISTA* lista_cruz_get(LISTA_CRUZ*, int lin, int col);

bool lista_cruz_vazia(LISTA_CRUZ*);

#endif // !LISTA_CRUZ_H
