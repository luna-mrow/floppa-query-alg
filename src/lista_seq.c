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
  if (tam_max < 1) tam_max = 16;

  LISTA_SEQ *lista = (LISTA_SEQ*) malloc(sizeof(LISTA_SEQ));
  if (lista == NULL) return NULL;
  lista->lista = (ITEM**) malloc(sizeof(ITEM*) * tam_max);
  if (lista->lista == NULL) { free(lista); return NULL; }

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

void lista_seq_liberar(LISTA_SEQ **lista) {
  if (lista == NULL || *lista == NULL) return;
  free((*lista)->lista);
  free(*lista);
  *lista = NULL;
}

int lista_seq_percorrer(LISTA_SEQ *lista, VISITA_ITEM visita, void *ctx) {
  if (lista == NULL || visita == NULL) return 0;

  for (int i = 0; i < lista->tamanho; i++) {
    if (visita(lista->lista[i], ctx)) return 1;
  }

  return 0;
}

bool lista_seq_inserir(LISTA_SEQ* lista, ITEM* item, int pos) {
  if (lista == NULL || item == NULL ||
      pos > lista->tamanho || (pos < 0 && -(long)pos > lista->tamanho + 1)) return false;
  if (lista->tamanho >= lista->tam_max) {
    int novo_max = lista->tam_max * 2;
    ITEM **novo = realloc(lista->lista, sizeof(ITEM*) * (size_t)novo_max);
    if (!novo) return false;
    lista->lista = novo;
    lista->tam_max = novo_max;
  }
  if (pos < 0) pos += lista->tamanho + 1;

  for (int i = lista->tamanho; i > pos; i--) lista->lista[i] = lista->lista[i - 1];
  lista->lista[pos] = item;
  lista->tamanho++;

  return true;
}

bool lista_seq_inserir_ordenado(LISTA_SEQ* lista, ITEM* item, const char *campo, int ordem) {
  if (lista == NULL || item == NULL || campo == NULL) return false;
  double val;
  if (!item_get_num(item, campo, &val)) return false;
  int lo = 0, hi = lista->tamanho;
  while (lo < hi) {
    int mid = lo + (hi - lo) / 2;
    double atual;
    if (!item_get_num(lista->lista[mid], campo, &atual)) return false;
    if (ordem ? atual > val : atual < val) lo = mid + 1;
    else hi = mid;
  }
  return lista_seq_inserir(lista, item, lo);
}

bool lista_seq_inserir_ordenado_sim(LISTA_SEQ *lista, ITEM *item, int desc) {
  if (!lista || !item) return false;
  int lo=0, hi=lista->tamanho;
  while (lo<hi) {
    int mid=lo+(hi-lo)/2;
    double atual=item_get_sim(lista->lista[mid]), novo=item_get_sim(item);
    if (desc ? atual > novo : atual < novo) lo=mid+1;
    else hi=mid;
  }
  return lista_seq_inserir(lista,item,lo);
}

bool lista_seq_inserir_ordenado_cmp(LISTA_SEQ *lista, ITEM *item,
                                    int (*comparar)(ITEM*, ITEM*, void*), void *ctx) {
  if (!lista || !item || !comparar) return false;
  int lo=0, hi=lista->tamanho;
  while (lo<hi) {
    int mid=lo+(hi-lo)/2;
    if (comparar(lista->lista[mid], item, ctx) <= 0) lo=mid+1;
    else hi=mid;
  }
  return lista_seq_inserir(lista,item,lo);
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
