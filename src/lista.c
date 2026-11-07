#include <stdio.h>
#include <stdlib.h>

#include "../headers/lista.h"

enum tipos_dado {
  TIPO_ATOMO, TIPO_LISTA
};

typedef union dado_ {
  ITEM *atomo;
  LISTA *sublista;
} DADO;

typedef struct no_ NO;

struct no_ {
  int tipo;
  DADO dado;
  NO *ante;
  NO *prox;
};

struct lista_ {
  NO *cabeca;
  NO *cauda;
};

LISTA* lista_criar() {
  LISTA *lista = (LISTA*) malloc(sizeof(LISTA));
  NO *cabeca = (NO*) malloc(sizeof(NO));
  NO *cauda = (NO*) malloc(sizeof(NO));
  if (lista == NULL || cabeca == NULL || cauda == NULL) return NULL;

  cabeca->tipo = TIPO_LISTA;
  cabeca->dado.sublista = NULL;
  cabeca->ante = NULL;
  cabeca->prox = cauda;

  cauda->tipo = TIPO_LISTA;
  cauda->dado.sublista = NULL;
  cauda->ante = cabeca;
  cauda->prox = NULL;

  lista->cabeca = cabeca;
  lista->cauda = cauda;

  return lista;
}

static void no_apagar(NO* no) {
  if (no->tipo == TIPO_LISTA) lista_apagar(&no->dado.sublista);
  else item_apagar(&no->dado.atomo);

  free(no);
}

void lista_apagar(LISTA **lista) {
  if (lista == NULL || *lista == NULL) return;

  NO *del = NULL, *prox = (*lista)->cabeca;

  while (prox != NULL) {
    del = prox;
    prox = prox->prox;

    no_apagar(del);
  }

  free(*lista);
  *lista = NULL;
}

//insercao: no depois do que sera inserido (dependente de direcao)
//remocao/busca: no correto ou a cabeca/cauda quando pos e grande demais
static NO* encontrar_pos(LISTA *lista, int pos) {
  bool pos_neg = pos < 0;
  if (pos_neg) pos = -(pos + 1); //regulariza -1 para 0 desde a cauda
  NO *prox = pos_neg ? lista->cauda->ante : lista->cabeca->prox;

  for (
    int i = 0;
    i < pos && prox != (pos_neg ? lista->cabeca : lista->cauda);
    i++, prox = (pos_neg ? prox->ante : prox->prox)
  );

  return prox;
}

static bool lista_inserir(LISTA *lista, int tipo, DADO dado, int pos) {
  if (lista == NULL || (tipo == TIPO_ATOMO ? dado.atomo == NULL : dado.sublista == NULL)) return false;

  NO *novo = malloc(sizeof(NO));
  if (novo == NULL) return false;

  novo->tipo = tipo;
  novo->dado = dado;

  //recalcular pos_neg e redundante mas necessario para a manipulacao de ponteiros
  bool pos_neg = pos < 0;
  NO* prox = encontrar_pos(lista, pos);

  if (pos_neg) {
    prox->prox->ante = novo;
    novo->prox = prox->prox;
    prox->prox = novo;
    novo->ante = prox;
  } else {
    prox->ante->prox = novo;
    novo->ante = prox->ante;
    prox->ante = novo;
    novo->prox = prox;
  }

  return true;
}

bool lista_inserir_atomo(LISTA* lista, ITEM* atomo, int pos) {
  DADO dado; dado.atomo = atomo;
  return lista_inserir(lista, TIPO_ATOMO, dado, pos);
}

bool lista_inserir_sublista(LISTA *lista, LISTA *sublista, int pos) {
  DADO dado; dado.sublista = sublista;
  return lista_inserir(lista, TIPO_LISTA, dado, pos);
}

bool lista_remover(LISTA* lista, int pos) {
  if (lista == NULL) return false;

  NO* del = encontrar_pos(lista, pos);

  //pos fora dos bounds (necessario, a lista vazia e um caso de borda para remocao)
  if (del == lista->cabeca || del == lista->cauda) return false;

  del->ante->prox = del->prox;
  del->prox->ante = del->ante;

  no_apagar(del);
  return true;
}

int lista_get_tipo(LISTA *lista, int pos) {
  if (lista == NULL) return -1;

  NO* no = encontrar_pos(lista, pos);

  return (no != lista->cabeca && no != lista->cauda) ? no->tipo : -1;
}

//nas funções get, o caso de erro para o programa, pois sinto que NULL e util como retorno

ITEM* lista_get_atomo(LISTA *lista, int pos) {
  if (lista == NULL) exit(EXIT_FAILURE);

  NO* no = encontrar_pos(lista, pos);

  //o tipo dos nos sentinela e TIPO_LISTA, entao filtrar por TIPO_ATOMO ja lida com este caso de borda
  return no->tipo == TIPO_ATOMO ? no->dado.atomo : NULL;
}

LISTA* lista_get_sublista(LISTA* lista, int pos) {
  if (lista == NULL) exit(EXIT_FAILURE);

  NO* no = encontrar_pos(lista, pos);

  return no->tipo == TIPO_LISTA ? ((no != lista->cabeca && no != lista->cauda) ? no->dado.sublista : NULL) : NULL;
}

static void lista_imprimir_rec(LISTA* lista, int profundidade) {
  if (lista == NULL) {
    printf("(nil)\n");
    return;
  }
  printf("(");

  if (profundidade == 0) printf("...");
  else {
    for (NO *aux = lista->cabeca->prox; aux != lista->cauda; aux = aux->prox) {
      if (aux->tipo == TIPO_ATOMO) printf("%d", item_get_id(aux->dado.atomo));
      else lista_imprimir_rec(aux->dado.sublista, profundidade - 1);
      if (aux->prox != lista->cauda) printf(",");
    }
  }

  printf(")");
}

//0 para profundidade maxima
void lista_imprimir(LISTA* lista, unsigned short profundidade) {
  if (profundidade == 0) profundidade = -1;
  lista_imprimir_rec(lista, profundidade);
  printf("\n");
}

//todas as funcoes de alias aqui em frente:

bool pilha_empilhar(LISTA* lista, ITEM* item) {
  return lista_inserir_atomo(lista, item, 0);
}

ITEM* pilha_desempilhar(LISTA* lista) {
  ITEM *atomo = lista_get_atomo(lista, 0);
  ITEM *res = item_copiar(atomo);

  lista_remover(lista, 0);
  return res;
}

bool fila_inserir(LISTA* lista, ITEM* item) {
  return lista_inserir_atomo(lista, item, -1);
}

//fila_remover() faz LITERALMENTE a mesma coisa que pilha_desempilhar()
//reutilizar parece valido?
ITEM* fila_remover(LISTA* lista) {
  return pilha_desempilhar(lista);
}
