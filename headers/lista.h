#ifndef LISTA_H
#define LISTA_H

#include <stdbool.h>
#include "item.h"

typedef struct lista_ LISTA;

LISTA* lista_criar();
void lista_apagar(LISTA**);

//pos negativa para propagar da cauda
bool lista_inserir_atomo(LISTA*, ITEM*, int pos);
bool lista_inserir_sublista(LISTA*, LISTA*, int pos);
bool lista_inserir_agrupado(LISTA*, ITEM*, const char *campos[], int n_campos);

bool lista_remover(LISTA*, int pos);

int lista_get_tipo(LISTA*, int pos);
ITEM* lista_get_atomo(LISTA*, int pos);
LISTA* lista_get_sublista(LISTA*, int pos);

bool lista_vazia(LISTA*);
int lista_tamanho(LISTA*);
//0 e para profundidade maxima (65535)
//qualquer numero acima de 65535 seria overkill TwT
void lista_imprimir(LISTA*, unsigned short profundidade);

//todos os proximos metodos sao aliases, atuando como cada uma das estruturas especificadas

bool pilha_empilhar(LISTA*, ITEM*);
ITEM* pilha_desempilhar(LISTA*);

bool fila_inserir(LISTA*, ITEM*);
ITEM* fila_remover(LISTA*);

#endif // !LISTA_H
