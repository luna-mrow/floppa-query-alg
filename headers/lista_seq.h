#ifndef LISTA_SEQ_H
#define LISTA_SEQ_H

#include <stdbool.h>
#include "item.h"

typedef struct lista_seq_ LISTA_SEQ;

LISTA_SEQ* lista_seq_criar(int tam_max);
void lista_seq_apagar(LISTA_SEQ**);
void lista_seq_liberar(LISTA_SEQ**);

//pos negativa para acessar do final
//lista_seq_inserir(-1) = O(1);
//lista_seq_remover(-1) = O(1);
bool lista_seq_inserir(LISTA_SEQ*, ITEM*, int pos);
//ASC = 0, DESC = 1
bool lista_seq_inserir_ordenado(LISTA_SEQ*, ITEM*, const char *campo, int ordem);
bool lista_seq_inserir_ordenado_sim(LISTA_SEQ*, ITEM*, int desc);
bool lista_seq_inserir_ordenado_cmp(LISTA_SEQ*, ITEM*, int (*comparar)(ITEM*, ITEM*, void*), void *ctx);

bool lista_seq_remover(LISTA_SEQ*, int pos);

ITEM* lista_seq_get(LISTA_SEQ*, int pos);

//percorre do indice 0 ate o ultimo (na lista ordenada, e a ordem de insercao ordenada)
//retorna 1 se o visitante interrompeu o percurso, 0 se percorreu tudo
int lista_seq_percorrer(LISTA_SEQ*, VISITA_ITEM, void *ctx);

//-1 para retornos de erro
int lista_seq_vazia(LISTA_SEQ*);
int lista_seq_cheia(LISTA_SEQ*);
int lista_seq_tamanho(LISTA_SEQ*);
void lista_seq_imprimir(LISTA_SEQ*);

#endif // !LISTA_SEQ_H
