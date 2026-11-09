#ifndef LISTA_CRUZ_H
#define LISTA_CRUZ_H

#include <stdbool.h>
#include "lista.h"

typedef struct lista_cruz_ LISTA_CRUZ;

LISTA_CRUZ* lista_cruz_criar();
void lista_cruz_apagar(LISTA_CRUZ**);
void lista_cruz_liberar(LISTA_CRUZ**);

bool lista_cruz_inserir(LISTA_CRUZ*, ITEM*, const char *lin_campo, const char *col_campo);

//uma desvantagem é que ao invés de remover um item específico, remove todos os itens em uma certa posição
bool lista_cruz_remover(LISTA_CRUZ*, int lin, int col);

LISTA* lista_cruz_get(LISTA_CRUZ*, int lin, int col);

//percorre pela primeira organizacao (linhas): linha por linha e, dentro de cada uma, coluna por coluna
//retorna 1 se o visitante interrompeu o percurso, 0 se percorreu tudo
int lista_cruz_percorrer(LISTA_CRUZ*, VISITA_ITEM, void *ctx);
//percorre pela segunda organizacao (colunas): coluna por coluna e, dentro de cada uma, linha por linha
int lista_cruz_percorrer_colunas(LISTA_CRUZ*, VISITA_ITEM, void *ctx);

bool lista_cruz_vazia(LISTA_CRUZ*);

#endif // !LISTA_CRUZ_H
