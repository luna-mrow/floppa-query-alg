#ifndef ARQUIVO_H
#define ARQUIVO_H

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

#include "macros.h"

bool arquivo_carregar(
    const char *caminho,
    Estrutura *base,
    size_t limite,
    size_t *quantidade,
    LEITOR_ITEM leitor,
    INSERIR_ITEM inserir
);

bool arquivo_escrever(
    const char *entrada,
    const char *saida,
    Estrutura *resultado,
    size_t top_k,
    size_t *quantidade,
    ESCRITOR_ITEM escritor
);

bool arquivo_inserir_lista(LISTA *estrutura, ITEM *item);
bool arquivo_inserir_pilha(LISTA *estrutura, ITEM *item);
bool arquivo_inserir_fila(LISTA *estrutura, ITEM *item);

#endif
