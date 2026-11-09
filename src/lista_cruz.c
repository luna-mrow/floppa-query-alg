#include <stdlib.h>

#include "../headers/lista_cruz.h"

typedef struct celula_ CELULA;

//cada celula tem uma sublista porque cada celula pode ter multiplos itens, todos com os valores cruzados iguais
struct celula_ {
  int lin;
  int col;
  LISTA* sublista;
  CELULA* direita;
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
  lista->cabeca->direita = NULL;
  lista->cabeca->abaixo = NULL;
  return lista;
}

void lista_cruz_apagar(LISTA_CRUZ **lista) {
  if (lista == NULL || *lista == NULL) return;

  CELULA *del = NULL, *prox = ((*lista)->cabeca), *aux = NULL;

  while (prox != NULL) {
      aux = prox;
      prox = prox->abaixo;

    while (aux != NULL) {
      del = aux;
      aux = aux->direita;

      if (del->sublista != NULL) lista_apagar(&del->sublista);
      free(del);
    }
  }

  free(*lista);
  *lista = NULL;
}

//para encontrar se uma linha ou coluna existe, coloque -1 no respectivo
static CELULA* celula_encontrar(LISTA_CRUZ* lista, int lin, int col) {
  CELULA *celula = lista->cabeca;

  while (celula->lin != lin) {
    if (celula->lin > lin || celula->abaixo == NULL) return NULL;
    celula = celula->abaixo;
  }
  while (celula->col != col) {
    if (celula->col > col || celula->abaixo == NULL) return NULL;
    celula = celula->direita;
  }
  
  return celula;
}

static bool celula_inserir(LISTA_CRUZ *lista, CELULA *celula) {
  CELULA *ante;

  if (celula->lin >= 0) {
    ante = celula_encontrar(lista, -1, celula->col);
    if (ante == NULL) return false;

    while (ante->abaixo != NULL && ante->abaixo->lin < celula->lin) ante = ante->abaixo;

    celula->abaixo = ante->abaixo;
    ante->abaixo = celula;
  }
  if (celula->col >= 0) {
    ante = celula_encontrar(lista, celula->lin, -1);
    if (ante == NULL) return false;

    while (ante->direita != NULL && ante->direita->col < celula->col) ante = ante->direita;

    celula->direita = ante->direita;
    ante->direita = celula;
  }

  return true;
}

bool lista_cruz_inserir(LISTA_CRUZ* lista, ITEM* item,  const char *lin_campo, const char *col_campo) {
  if (lista == NULL || item == NULL || lin_campo == NULL || col_campo == NULL) return NULL;

  //double mas terá typecast para int quando necessario
  double lin, col;
  if (item_get_num(item, lin_campo, &lin) == 0) return false;
  if (item_get_num(item, col_campo, &col) == 0) return false;
  if (lin < 0 || col < 0) return false;

  CELULA *pos = celula_encontrar(lista, (int) lin, (int) col);
  if (pos == NULL) {
    CELULA *nova;
    if (celula_encontrar(lista, (int) lin, -1) == NULL) {
      nova = (CELULA*) malloc(sizeof(CELULA));
      if (nova == NULL) return false;

      nova->lin = (int) lin;
      nova->col = -1;
      nova->sublista = NULL;

      if (!celula_inserir(lista, nova)) return false;
    }
    if (celula_encontrar(lista, -1, (int) col) == NULL) {
      nova = (CELULA*) malloc(sizeof(CELULA));
      if (nova == NULL) return false;

      nova->lin = -1;
      nova->col = (int) col;
      nova->sublista = NULL;

      if (!celula_inserir(lista, nova)) return false;
    }

    nova = (CELULA*) malloc(sizeof(CELULA));
    if (nova == NULL) return false;

    nova->lin = (int) lin;
    nova->col = (int) col;
    nova->sublista = lista_criar();

    if (!celula_inserir(lista, nova)) return false;
    pos = nova;
  }

  return lista_inserir_atomo(pos->sublista, item, -1);
}

static bool celula_remover(LISTA_CRUZ* lista, int lin, int col) {
  CELULA *ante, *del;

  if (col != -1) {
    ante = celula_encontrar(lista, -1, col);
    while (ante->abaixo->lin != lin) {
      if (ante->abaixo == NULL || ante->abaixo->lin > lin) return false;
      ante = ante->abaixo;
    }
    del = ante->abaixo;
    ante->abaixo = del->abaixo;

    lista_apagar(&del->sublista);
    free(del);
    if (lin == -1) return true;
  }

  if (lin != -1) {
    ante = celula_encontrar(lista, lin, -1);
    while (ante->direita->col != col) {
      if (ante->direita == NULL || ante->direita->col > col) return false;
      ante = ante->direita;
    }
    del = ante->direita;
    ante->direita = del->direita;

    lista_apagar(&del->sublista);
    free(del);
    return true; //aqui pode retornar direto, pois a celula foi removida das duas direcoes
  }

  //falso porque a cabeca nao pode ser removida
  return false;
}

bool lista_cruz_remover(LISTA_CRUZ* lista, int lin, int col) {
  if (lin < 0 || col < 0 || celula_encontrar(lista, lin, col) == NULL) return false;

  celula_remover(lista, lin, col);

  //libera as linhas e colunas se elas não apontam para nada;
  CELULA* verif = celula_encontrar(lista, lin, -1);
  if (verif->direita == NULL) celula_remover(lista, lin, -1);
  verif = celula_encontrar(lista, -1, col);
  if (verif->abaixo == NULL) celula_remover(lista, -1, col);

  return true;
}

LISTA* lista_cruz_get(LISTA_CRUZ *lista, int lin, int col) {
  if (lista == NULL || lin < 0 || col < 0) return NULL;

  CELULA* celula = celula_encontrar(lista, lin, col);

  return celula != NULL ? celula->sublista : NULL;
}

bool lista_cruz_vazia(LISTA_CRUZ *lista) {
  return lista != NULL ? lista->cabeca->abaixo == NULL && lista->cabeca->direita == NULL : false;
}
