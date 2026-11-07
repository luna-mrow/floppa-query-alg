#include <stdlib.h>

#include "../headers/lista_cruz.h"
#include "../headers/lista.h"
#include "../headers/macros.h"

typedef struct celula_ CELULA;

//cada celula tem uma sublista porque cada celula pode ter multiplos itens, todos com os valores cruzados iguais
struct celula_ {
  int lin;
  int col;
  LISTA* sublista;
  CELULA* esquerda;
  CELULA* abaixo;
};

struct lista_cruz_ {
  CELULA *cabeca;
};

LISTA_CRUZ* lista_cruz_criar() {
  LISTA_CRUZ *lista = (LISTA_CRUZ*) malloc(sizeof(LISTA_CRUZ));
  if (lista == NULL) return NULL;

  lista->cabeca = (CELULA*) malloc(sizeof(CELULA));
  if (lista->cabeca == NULL) return NULL;

  lista->cabeca->lin = -1;
  lista->cabeca->col = -1;
  lista->cabeca->sublista = NULL;
  lista->cabeca->esquerda = NULL;
  lista->cabeca->abaixo = NULL;
  return lista;
}

void lista_cruz_apagar(LISTA_CRUZ **lista) {
  if (lista == NULL || *lista == NULL) return;

  CELULA *del = NULL, *prox = (*lista->cabeca), *aux = NULL;

  while (prox != NULL) {
      aux = prox;
      prox = prox->abaixo;

    while (aux != NULL) {
      del = aux;
      aux = aux->esquerda;

      if (del->sublista != NULL) lista_apagar(&del->sublista);
      free(del);
    }
  }

  free(*lista);
  *lista = NULL;
}

bool lista_cruz_inserir(LISTA_CRUZ *lista_cruz, ITEM *item, int lin, int col) {
  if (lista_cruz == NULL || item == NULL || lin < 0 || col < 0) return NULL;

  if (item_get(item) == 0) {
    ITEM *aux = lista_cruz_remove(lista_cruz, lin, col);
    item_delete(&aux);
    return true;
  }

  CELULA *new = malloc(sizeof(CELULA));
  if (new == NULL) return false;

  new->lin = lin;
  new->col = col;
  new->item = item_create(item_get(item));
  new->esquerda = NULL;
  new->abaixo = NULL;

  CELULA *prev = NULL, *next = lista_cruz->lins[lin];
  while (next != NULL) {
    if (next->col < col) {
      prev = next;
      next = next->esquerda;
    } else if (next->col == col && next->lin == lin) {
      item_delete(&next->item);
      next->item = item;
      return true;
    } else {
      if (prev == NULL) lista_cruz->lins[lin] = new;
      else prev->esquerda = new;

      new->esquerda = next;
    }
  }

  if (next == NULL) {
    if (prev == NULL) lista_cruz->lins[lin] = new;
    else prev->esquerda = new;
  }

  prev = NULL, next = lista_cruz->cols[col];
  while (next != NULL) {
    if (next->lin < lin) {
      prev = next;
      next = next->abaixo;
    } else if (next->lin == lin && next->col == col) {
      item_delete(&next->item);
      next->item = item;
      return true;
    } else {
      if (prev == NULL) lista_cruz->cols[col] = new;
      else prev->abaixo = new;

      new->abaixo = next;
    }
  }

  if (next == NULL) {
    if (prev == NULL) lista_cruz->cols[col] = new;
    else prev->abaixo = new;
  }

  return true;
}

ITEM* lista_cruz_remove(LISTA_CRUZ* lista_cruz, int lin, int col) {

  CELULA *prev = NULL, *aux = lista_cruz->lins[lin];

  if (aux != NULL) {
    while (aux->esquerda != NULL && aux->esquerda->col <= col) {
      prev = aux;
      aux = aux->esquerda;
    }

    if (aux->col == col) {
      if (prev == NULL) lista_cruz->lins[lin] = aux->esquerda;
      else prev->esquerda = aux->esquerda;

      prev = NULL, aux = lista_cruz->cols[col];
      while (aux->abaixo != NULL && aux->abaixo->lin <= lin) {
        prev = aux;
        aux = aux->abaixo;
      }

      if (prev == NULL) lista_cruz->cols[col] = aux->abaixo;
      else prev->abaixo = aux->abaixo;
      
      ITEM *res = aux->item;
      free(aux);
      return res;
    }
  }

  return item_create(0);
}

ITEM* lista_cruz_get(LISTA_CRUZ *lista_cruz, int lin, int col) {
  if (lista_cruz == NULL || lin >= lista_cruz->lin_n || col >= lista_cruz->col_n) return NULL;

  CELULA *aux = lista_cruz->lins[lin];

  if (aux != NULL) {
    while (aux->esquerda != NULL && aux->esquerda->col <= col) aux = aux->esquerda;

    if (aux->col == col) return item_create(item_get(aux->item));
  }

  return item_create(0);
}
