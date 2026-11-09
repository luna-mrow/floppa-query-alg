#ifndef ARQUIVO_H
#define ARQUIVO_H

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

#include "def.h"

bool arquivo_carregar(
    const char *caminho,
    Estrutura *base,
    size_t limite,
    size_t *quantidade,
    LEITOR_ITEM leitor,
    Query *query
);

bool arquivo_escrever(
    const char *entrada,
    const char *saida,
    Estrutura *resultado,
    size_t top_k,
    size_t *quantidade,
    ESCRITOR_ITEM escritor
);

bool arquivo_inserir_pilha(Estrutura *estrutura, ITEM *item, Query *query);
bool arquivo_inserir_fila(Estrutura *estrutura, ITEM *item, Query *query);
bool arquivo_inserir_lista(Estrutura *estrutura, ITEM *item, Query *query);
bool arquivo_inserir_seq(Estrutura *estrutura, ITEM *item, Query *query);
bool arquivo_inserir_ordenado(Estrutura *estrutura, ITEM *item, Query *query);
bool arquivo_inserir_agrupado(Estrutura *estrutura, ITEM *item, Query *query);
bool arquivo_inserir_cruzado(Estrutura *estrutura, ITEM *item, Query *query);

#endif
