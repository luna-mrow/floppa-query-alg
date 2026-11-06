#ifndef ARQUIVO_H
#define ARQUIVO_H

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

#include "lista.h"

// Ponteiro para uma função que lê uma linha do arquivo e transforma essa linha em um ITEM.
// Modificar conforme item.c
typedef ITEM* (*LEITOR_ITEM)(FILE *arquivo);

// Ponteiro para uma função que escreve um ITEM no arquivo no formato do dataset.
// Modificar conforme item.c
typedef bool (*ESCRITOR_ITEM)(FILE *arquivo, ITEM *item);

// Ponteiro para uma função que insere um ITEM na estrutura escolhida.
// Modificar conforme item.c
typedef bool (*INSERIR_ITEM)(LISTA *estrutura, ITEM *item);


// Carrega os registros de um arquivo para uma estrutura.
/*
    caminho: caminho do arquivo de entrada.
    base: estrutura que receberá os registros.
    limite: quantidade máxima de registros. 0 significa carregar todos.
    quantidade: recebe a quantidade de registros carregados.
    leitor: função responsável por interpretar cada linha.
    inserir: função responsável por inserir o ITEM na estrutura.
*/
bool arquivo_carregar(
    const char *caminho,
    LISTA *base,
    size_t limite,
    size_t *quantidade,
    LEITOR_ITEM leitor,
    INSERIR_ITEM inserir
);


// Escreve os resultados em um arquivo.
/*
    entrada: arquivo original, utilizado para recuperar o cabeçalho.
    saida: caminho do arquivo de saída.
    resultado: estrutura que contém os resultados da busca.
    top_k: quantidade máxima de registros exportados. 0 significa exportar todos.
    quantidade: recebe a quantidade de registros escritos.
    escritor: função responsável por escrever cada ITEM.
*/
bool arquivo_escrever(
    const char *entrada,
    const char *saida,
    LISTA *resultado,
    size_t top_k,
    size_t *quantidade,
    ESCRITOR_ITEM escritor
);


// Funções de inserção para as estruturas já disponíveis.
bool arquivo_inserir_lista(LISTA *estrutura, ITEM *item);
bool arquivo_inserir_pilha(LISTA *estrutura, ITEM *item);
bool arquivo_inserir_fila(LISTA *estrutura, ITEM *item);

#endif