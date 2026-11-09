#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
  char **lin_chaves;
  int n_linhas;
  char **col_chaves;
  int n_colunas;
};

LISTA_CRUZ* lista_cruz_criar() {
  LISTA_CRUZ *lista = (LISTA_CRUZ*) malloc(sizeof(LISTA_CRUZ));
  if (lista == NULL) return NULL;

  lista->cabeca = (CELULA*) malloc(sizeof(CELULA));
  if (lista->cabeca == NULL) { free(lista); return NULL; }

  lista->cabeca->lin = -1;
  lista->cabeca->col = -1;
  lista->cabeca->sublista = NULL;
  lista->cabeca->direita = NULL;
  lista->cabeca->abaixo = NULL;
  lista->lin_chaves = NULL; lista->n_linhas = 0;
  lista->col_chaves = NULL; lista->n_colunas = 0;
  return lista;
}

static void lista_cruz_destruir(LISTA_CRUZ **lista, int apagar_itens) {
  if (lista == NULL || *lista == NULL) return;
  CELULA *cab = (*lista)->cabeca;
  CELULA *linha = cab->abaixo;
  while (linha != NULL) {
    CELULA *prox_linha = linha->abaixo;
    CELULA *cel = linha->direita;
    while (cel != NULL) {
      CELULA *prox = cel->direita;
      if (cel->sublista) {
        if (apagar_itens) lista_apagar(&cel->sublista);
        else lista_liberar_nos(&cel->sublista);
      }
      free(cel);
      cel = prox;
    }
    free(linha);
    linha = prox_linha;
  }
  CELULA *col = cab->direita;
  while (col != NULL) { CELULA *prox = col->direita; free(col); col = prox; }
  free(cab);
  for (int i=0; i<(*lista)->n_linhas; i++) free((*lista)->lin_chaves[i]);
  for (int i=0; i<(*lista)->n_colunas; i++) free((*lista)->col_chaves[i]);
  free((*lista)->lin_chaves); free((*lista)->col_chaves);
  free(*lista);
  *lista = NULL;
}
void lista_cruz_apagar(LISTA_CRUZ **lista) { lista_cruz_destruir(lista, 1); }
void lista_cruz_liberar(LISTA_CRUZ **lista) { lista_cruz_destruir(lista, 0); }

//para encontrar se uma linha ou coluna existe, coloque -1 no respectivo
static CELULA* celula_encontrar(LISTA_CRUZ* lista, int lin, int col) {
  CELULA *celula = lista->cabeca;

  while (celula->lin != lin) {
    if (celula->lin > lin || celula->abaixo == NULL) return NULL;
    celula = celula->abaixo;
  }
  while (celula->col != col) {
    if (celula->col > col || celula->direita == NULL) return NULL;
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

static int indice_chave(char ***chaves, int *n, const char *valor) {
  for (int i=0; i<*n; i++) if (strcmp((*chaves)[i], valor)==0) return i;
  char **novo = realloc(*chaves, sizeof(char*) * (size_t)(*n + 1));
  if (!novo) return -1;
  *chaves = novo;
  novo[*n] = malloc(strlen(valor)+1);
  if (!novo[*n]) return -1;
  strcpy(novo[*n], valor);
  return (*n)++;
}

static int valor_indice(ITEM *item, const char *campo, char ***chaves, int *n) {
  const char *valor = item_get_str(item, campo);
  char buffer[64];
  if (!valor) {
    double numero;
    if (!item_get_num(item, campo, &numero)) return -1;
    snprintf(buffer, sizeof buffer, "%.15g", numero);
    valor = buffer;
  }
  return indice_chave(chaves, n, valor);
}

bool lista_cruz_inserir(LISTA_CRUZ* lista, ITEM* item,  const char *lin_campo, const char *col_campo) {
  if (lista == NULL || item == NULL || lin_campo == NULL || col_campo == NULL) return NULL;

  int lin = valor_indice(item, lin_campo, &lista->lin_chaves, &lista->n_linhas);
  int col = valor_indice(item, col_campo, &lista->col_chaves, &lista->n_colunas);
  if (lin < 0 || col < 0) return false;

  CELULA *pos = celula_encontrar(lista, lin, col);
  if (pos == NULL) {
    CELULA *nova;
    if (celula_encontrar(lista, lin, -1) == NULL) {
      nova = (CELULA*) malloc(sizeof(CELULA));
      if (nova == NULL) return false;

      nova->lin = lin;
      nova->col = -1;
      nova->sublista = NULL;
      nova->abaixo = NULL;
      nova->direita = NULL;

      if (!celula_inserir(lista, nova)) return false;
    }
    if (celula_encontrar(lista, -1, col) == NULL) {
      nova = (CELULA*) malloc(sizeof(CELULA));
      if (nova == NULL) return false;

      nova->lin = -1;
      nova->col = col;
      nova->sublista = NULL;
      nova->abaixo = NULL;
      nova->direita = NULL;

      if (!celula_inserir(lista, nova)) return false;
    }

    nova = (CELULA*) malloc(sizeof(CELULA));
    if (nova == NULL) return false;

    nova->lin = lin;
    nova->col = col;
    nova->sublista = lista_criar();
    nova->abaixo = NULL;
    nova->direita = NULL;

    if (!celula_inserir(lista, nova)) return false;
    pos = nova;
  }

  return lista_inserir_atomo(pos->sublista, item, -1);
}

static bool celula_remover(LISTA_CRUZ *lista, int lin, int col) {
  if (!lista || (lin < 0 && col < 0)) return false;
  if (lin == -1) {
    CELULA *ant = lista->cabeca;
    while (ant->direita && ant->direita->col < col) ant = ant->direita;
    if (!ant->direita || ant->direita->col != col) return false;
    CELULA *del = ant->direita;
    ant->direita = del->direita;
    free(del);
    return true;
  }
  if (col == -1) {
    CELULA *ant = lista->cabeca;
    while (ant->abaixo && ant->abaixo->lin < lin) ant = ant->abaixo;
    if (!ant->abaixo || ant->abaixo->lin != lin) return false;
    CELULA *del = ant->abaixo;
    ant->abaixo = del->abaixo;
    free(del);
    return true;
  }

  CELULA *linha = celula_encontrar(lista, lin, -1);
  CELULA *coluna = celula_encontrar(lista, -1, col);
  if (!linha || !coluna) return false;
  CELULA *ant_l = linha;
  while (ant_l->direita && ant_l->direita->col < col) ant_l = ant_l->direita;
  if (!ant_l->direita || ant_l->direita->col != col) return false;
  CELULA *cel = ant_l->direita;
  CELULA *ant_c = coluna;
  while (ant_c->abaixo && ant_c->abaixo->lin < lin) ant_c = ant_c->abaixo;
  if (!ant_c->abaixo || ant_c->abaixo != cel) return false;
  ant_l->direita = cel->direita;
  ant_c->abaixo = cel->abaixo;
  lista_apagar(&cel->sublista);
  free(cel);
  return true;
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

//as linhas ficam encadeadas por "abaixo" a partir da cabeca e cada linha encadeia suas celulas por "direita"
int lista_cruz_percorrer(LISTA_CRUZ *lista, VISITA_ITEM visita, void *ctx) {
  if (lista == NULL || visita == NULL) return 0;

  for (CELULA *lin = lista->cabeca->abaixo; lin != NULL; lin = lin->abaixo) {
    for (CELULA *cel = lin->direita; cel != NULL; cel = cel->direita) {
      if (lista_percorrer(cel->sublista, visita, ctx)) return 1;
    }
  }

  return 0;
}

//as colunas ficam encadeadas por "direita" a partir da cabeca e cada coluna encadeia suas celulas por "abaixo"
int lista_cruz_percorrer_colunas(LISTA_CRUZ *lista, VISITA_ITEM visita, void *ctx) {
  if (lista == NULL || visita == NULL) return 0;

  for (CELULA *col = lista->cabeca->direita; col != NULL; col = col->direita) {
    for (CELULA *cel = col->abaixo; cel != NULL; cel = cel->abaixo) {
      if (lista_percorrer(cel->sublista, visita, ctx)) return 1;
    }
  }

  return 0;
}

bool lista_cruz_vazia(LISTA_CRUZ *lista) {
  return lista != NULL ? lista->cabeca->abaixo == NULL && lista->cabeca->direita == NULL : false;
}
