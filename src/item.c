#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>

#include "../headers/item.h"

enum tipo_coluna { COL_TEXTO, COL_NUM, COL_ANO_DATA };

typedef struct {
  const char *nome;
  int tipo;
  int origem;
} COLUNA;

typedef struct {
  char delim;
  int n_arquivo;
  int n;
  const COLUNA *colunas;
} TEMPLATE;

#define TXT(nome) { nome, COL_TEXTO, -1 }
#define NUM(nome) { nome, COL_NUM, -1 }

static const COLUNA COLUNAS_NASA[] = {
  TXT("asteroid_designation"), TXT("asteroid_fullname"), TXT("close_approach_date"),
  NUM("year"), NUM("month"), NUM("distance_au"), NUM("velocity_km_s"),
  NUM("absolute_magnitude"), NUM("days_until_approach"),
  TXT("is_past_event"), TXT("is_future_event"),
  NUM("risk_score"), NUM("panic_level"),
  TXT("threat_category"), TXT("panic_verdict"), TXT("on_sentry_list"),
  NUM("sentry_impact_prob"), NUM("sentry_torino_scale"),
  NUM("sentry_palermo_scale"), NUM("sentry_diameter_km")
};

static const COLUNA COLUNAS_ANCINE[] = {
  TXT("NOME_SALA"), NUM("REGISTRO_SALA"), TXT("CNPJ_SALA"), TXT("SITUACAO_SALA"),
  TXT("DATA_SITUACAO_SALA"), TXT("DATA_INICIO_FUNCIONAMENTO_SALA"),
  NUM("ASSENTOS_SALA"), NUM("ASSENTOS_CADEIRANTES"),
  NUM("ASSENTOS_MOBILIDADE_REDUZIDA"), NUM("ASSENTOS_OBESIDADE"),
  TXT("ACESSO_ASSENTOS_COM_RAMPA"), TXT("ACESSO_SALA_COM_RAMPA"), TXT("BANHEIROS_ACESSIVEIS"),
  TXT("NOME_COMPLEXO"), NUM("REGISTRO_COMPLEXO"), TXT("SITUACAO_COMPLEXO"),
  TXT("DATA_SITUACAO_COMPLEXO"), TXT("PAGINA_ELETRONICA_COMPLEXO"),
  TXT("ENDERECO_COMPLEXO"), NUM("NUMERO_ENDERECO_COMPLEXO"), TXT("COMPLEMENTO_COMPLEXO"),
  TXT("BAIRRO_COMPLEXO"), TXT("MUNICIPIO_COMPLEXO"), TXT("CEP_COMPLEXO"), TXT("UF_COMPLEXO"),
  TXT("COMPLEXO_ITINERANTE"), TXT("OPERACAO_USUAL"),
  TXT("NOME_EXIBIDOR"), NUM("REGISTRO_EXIBIDOR"), TXT("CNPJ_EXIBIDOR"),
  TXT("SITUACAO_EXIBIDOR"), TXT("NOME_GRUPO_EXIBIDOR"),
  { "ano_inicio_sala", COL_ANO_DATA, 5 }
};

static const TEMPLATE TEMPLATE_NASA = { ',', 20, 20, COLUNAS_NASA };
static const TEMPLATE TEMPLATE_ANCINE = { ';', 32, 33, COLUNAS_ANCINE };

static const char VAZIO[] = "";
#define TAM_ANO 8

struct item_ {
  int id;
  double sim;
  int crlf;
  const TEMPLATE *tpl;
  char *memoria;
  const char **texto;
  double *num;
  unsigned long long tem_num;
};

static int proximo_id = 0;

static int dividir(char *s, char delim, char **campos, int max) {
  int n = 0, aspas = 0;
  char *w = s;

  campos[n++] = w;
  for (; *s != '\0'; s++) {
    if (*s == '"') {
      if (aspas && s[1] == '"') { *w++ = '"'; s++; }
      else aspas = !aspas;
    } else if (*s == delim && !aspas) {
      *w++ = '\0';
      if (n < max) campos[n++] = w;
    } else {
      *w++ = *s;
    }
  }
  *w = '\0';

  return n;
}

static int para_double(const char *s, double *out) {
  char *fim;

  if (*s == '\0') return 0;
  *out = strtod(s, &fim);
  while (*fim == ' ') fim++;

  return *fim == '\0' && isfinite(*out);
}

static int ano_de_data(const char *data, double *ano) {
  const char *barra = strrchr(data, '/');

  if (barra == NULL || strlen(barra + 1) != 4) return 0;
  return para_double(barra + 1, ano);
}

static ITEM* item_de_linha(const TEMPLATE *t, const char *linha, int crlf) {
  size_t len = strlen(linha);
  size_t n = (size_t) t->n;

  ITEM *item = (ITEM*) malloc(sizeof(ITEM) + n * sizeof(char*) + n * sizeof(double));
  if (item == NULL) return NULL;

  item->memoria = (char*) malloc(2 * (len + 1) + (n - (size_t) t->n_arquivo) * TAM_ANO);
  if (item->memoria == NULL) { free(item); return NULL; }

  item->texto = (const char**) (item + 1);
  item->num = (double*) (item->texto + n);
  item->tpl = t;
  item->crlf = crlf;
  item->sim = 0;
  item->id = ++proximo_id;
  item->tem_num = 0;

  char *copia = item->memoria;
  char *buffer = copia + len + 1;
  char *extra = buffer + len + 1;
  char *campos[64];

  strcpy(copia, linha);
  strcpy(buffer, linha);
  int lidos = dividir(buffer, t->delim, campos, t->n_arquivo);

  for (int i = 0; i < t->n; i++) {
    const COLUNA *col = &t->colunas[i];
    item->texto[i] = VAZIO;
    item->num[i] = 0;

    if (col->tipo == COL_ANO_DATA) {
      double ano;
      if (ano_de_data(item->texto[col->origem], &ano)) {
        sprintf(extra, "%d", (int) ano);
        item->texto[i] = extra;
        item->num[i] = ano;
        item->tem_num |= 1ULL << i;
        extra += TAM_ANO;
      }
      continue;
    }

    if (i < lidos) item->texto[i] = campos[i];
    if (col->tipo == COL_NUM && para_double(item->texto[i], &item->num[i]))
      item->tem_num |= 1ULL << i;
  }

  return item;
}

static ITEM* ler_item(FILE *arquivo, const TEMPLATE *t) {
  char *linha = NULL;
  size_t cap = 0;
  ssize_t len;
  ITEM *item = NULL;

  if (arquivo == NULL) return NULL;

  while ((len = getline(&linha, &cap, arquivo)) != -1) {
    int crlf = 0;

    if (len > 0 && linha[len - 1] == '\n') linha[--len] = '\0';
    if (len > 0 && linha[len - 1] == '\r') { linha[--len] = '\0'; crlf = 1; }
    if (len == 0) continue;

    item = item_de_linha(t, linha, crlf);
    break;
  }

  free(linha);
  return item;
}

static bool escrever_item(FILE *arquivo, ITEM *item) {
  if (arquivo == NULL || item == NULL) return false;

  return fputs(item->memoria, arquivo) != EOF &&
         fputs(item->crlf ? "\r\n" : "\n", arquivo) != EOF;
}

ITEM* item_ler_nasa(FILE *arquivo) { return ler_item(arquivo, &TEMPLATE_NASA); }
bool item_escrever_nasa(FILE *arquivo, ITEM *item) { return escrever_item(arquivo, item); }

ITEM* item_ler_ancine(FILE *arquivo) { return ler_item(arquivo, &TEMPLATE_ANCINE); }
bool item_escrever_ancine(FILE *arquivo, ITEM *item) { return escrever_item(arquivo, item); }

ITEM* item_copiar(ITEM *item) {
  if (item == NULL) return NULL;

  ITEM *copia = item_de_linha(item->tpl, item->memoria, item->crlf);
  if (copia == NULL) return NULL;

  copia->id = item->id;
  copia->sim = item->sim;
  return copia;
}

void item_apagar(ITEM **item) {
  if (item == NULL || *item == NULL) return;

  free((*item)->memoria);
  free(*item);
  *item = NULL;
}

static int indice_campo(ITEM *item, const char *campo) {
  for (int i = 0; i < item->tpl->n; i++)
    if (strcasecmp(item->tpl->colunas[i].nome, campo) == 0) return i;

  return -1;
}

int item_get_id(ITEM *item) {
  return item != NULL ? item->id : -1;
}

int item_get_num(ITEM *item, const char *campo, double *out) {
  if (item == NULL || campo == NULL || out == NULL) return 0;

  int i = indice_campo(item, campo);
  if (i < 0 || !((item->tem_num >> i) & 1ULL)) return 0;

  *out = item->num[i];
  return 1;
}

const char *item_get_str(ITEM *item, const char *campo) {
  if (item == NULL || campo == NULL) return NULL;

  int i = indice_campo(item, campo);
  if (i < 0 || item->texto[i][0] == '\0') return NULL;

  return item->texto[i];
}

void item_set_sim(ITEM *item, double sim) {
  if (item != NULL) item->sim = sim;
}

double item_get_sim(ITEM *item) {
  return item != NULL ? item->sim : 0;
}
