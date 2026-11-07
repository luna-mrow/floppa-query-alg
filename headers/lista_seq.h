#ifndef LISTA_SEQ_H
#define LISTA_SEQ_H

#include <stdbool.h>
#include "item.h"

typedef struct lista_seq_ LISTA_SEQ;

LISTA_SEQ* lista_seq_criar(int tam_max);
void lista_seq_apagar(LISTA_SEQ**);

//pos negativa para acessar do final
//lista_seq_inserir(-1) = O(1);
//lista_seq_remover(-1) = O(1);
bool lista_seq_inserir(LISTA_SEQ*, ITEM*, int pos);
//bool lista_seq_inserir_ordenado(LISTA_SEQ*, ITEM*, TEMPLATE*, char* nome_campo, char* ordenamento);

bool lista_seq_remover(LISTA_SEQ*, int pos);

ITEM* lista_seq_get(LISTA_SEQ*, int pos);

void lista_seq_imprimir(LISTA_SEQ*);

#endif // !LISTA_SEQ_H
