#include <stdio.h>
#include <stdlib.h>

#include "../headers/lista_seq.h"

struct lista_seq_ {
  ITEM** lista;
  int tamanho;
  int tam_max;
  bool ordenada;
};

LISTA_SEQ* lista_seq_criar(int tam_max) {
  if (tam_max < 1) return NULL;

  LISTA_SEQ *lista = (LISTA_SEQ*) malloc(sizeof(LISTA_SEQ));
  if (lista == NULL) return NULL;
  lista->lista = (ITEM**) malloc(sizeof(ITEM*) * tam_max);
  if (lista->lista == NULL) return NULL;

  lista->tamanho = 0;
  lista->tam_max = tam_max;
  lista->ordenada = true;

  return lista;
}

void lista_seq_apagar(LISTA_SEQ **lista) {
  if (lista == NULL || *lista == NULL) return;

  for (int i = 0; i < (*lista)->tamanho; i++) item_apagar(&(*lista)->lista[i]);
  free((*lista)->lista);
  free(*lista);
  *lista = NULL;
}

bool lista_seq_inserir(LISTA_SEQ* lista, ITEM* item, int pos) {
  if (
    lista == NULL || item == NULL ||
    lista->tamanho >= lista->tam_max ||
    pos > lista->tamanho || -pos > lista->tamanho + 1
  ) return false;

  if (pos < 0) pos += lista->tamanho + 1;

  for (int i = lista->tamanho; i > pos; i--) lista->lista[i] = lista->lista[i - 1];
  lista->lista[pos] = item;
  lista->tamanho++;

  return true;
}

bool lista_seq_inserir_ordenado(LISTA_SEQ* lista, ITEM* item, const char *campo, int ordem) {
  if (lista == NULL || item == NULL || lista->tamanho >= lista->tam_max) return false;

  double val, mid_val;
  if (item_get_num(item, campo, &val) == 0) return false;

  int bot = 0, top = lista->tamanho - 1, mid = -1; //mid = -1 para lista vazia (insere no final)
  while (bot <= top) {
    mid = bot + (top - bot)/2;
    if (item_get_num(lista->lista[mid], campo, &mid_val) == 0) return false;

    if (val == mid_val) break;
    if (ordem ? val > mid_val : val < mid_val) top = mid - 1;
    else bot = mid + 1;
  }
  
  //este mid++ garante casos de borda (incluindo final da lista)
  //mid tem viés para baixo por causa do floor de divisão de inteiros
  if (ordem ? val < mid_val : val > mid_val) mid++; 
  return lista_seq_inserir(lista, item, mid);
}

bool lista_seq_remover(LISTA_SEQ* lista, int pos) {
  if (lista == NULL || pos >= lista->tamanho || -pos >= lista->tamanho + 1) return false;

  if (pos < 0) pos += lista->tamanho;

  for (int i = pos; i < lista->tamanho - 1; i++) lista->lista[i] = lista->lista[i + 1];
  lista->tamanho--;

  return true;
}

ITEM* lista_seq_get(LISTA_SEQ* lista, int pos) {
  if (
    lista == NULL || pos >= lista->tamanho || -pos >= lista->tamanho + 1) return NULL;

  if (pos < 0) pos += lista->tamanho;
  
  return lista->lista[pos];
}

int lista_seq_vazia(LISTA_SEQ *lista) {
  return lista != NULL ? lista->tamanho == 0 : -1;
}

int lista_seq_cheia(LISTA_SEQ *lista) {
  return lista != NULL ? lista->tamanho == lista->tam_max : -1;
}

int lista_seq_tamanho(LISTA_SEQ *lista) {
  return lista != NULL ? lista->tamanho : -1;
}

void lista_seq_imprimir(LISTA_SEQ* lista) {
  if (lista == NULL || lista->tamanho == 0) return;

  printf("%d", item_get_id(lista->lista[0]));
  for (int i = 1; i < lista->tamanho; i++) printf(", %d", item_get_id(lista->lista[i]));
}
