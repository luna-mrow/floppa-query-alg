#ifndef ARQUIVO_H
#define ARQUIVO_H

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

#include "lista.h"

// Modificar conforme item.c
typedef ITEM* (*LEITOR_ITEM)(FILE *arquivo);

// Modificar conforme item.c
typedef bool (*ESCRITOR_ITEM)(FILE *arquivo, ITEM *item);

// Modificar conforme item.c
typedef bool (*INSERIR_ITEM)(LISTA *estrutura, ITEM *item);

bool arquivo_carregar(
    const char *caminho,
    LISTA *base,
    size_t limite,
    size_t *quantidade,
    LEITOR_ITEM leitor,
    INSERIR_ITEM inserir
);

bool arquivo_escrever(
    const char *entrada,
    const char *saida,
    LISTA *resultado,
    size_t top_k,
    size_t *quantidade,
    ESCRITOR_ITEM escritor
);


bool arquivo_inserir_lista(LISTA *estrutura, ITEM *item);
bool arquivo_inserir_pilha(LISTA *estrutura, ITEM *item);
bool arquivo_inserir_fila(LISTA *estrutura, ITEM *item);

#endif
