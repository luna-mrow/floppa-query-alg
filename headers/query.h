#ifndef QUERY_H
#define QUERY_H

#include <stddef.h>
#include "item.h"

#define Q_MAX    32
#define Q_NOME   64
#define Q_VALOR  128

typedef enum { OP_EQ, OP_NE, OP_LT, OP_LE, OP_GT, OP_GE } Operador;
typedef enum { VAL_NUM, VAL_BOOL, VAL_STR } TipoValor;

typedef struct {
    char      campo[Q_NOME];
    Operador  op;
    TipoValor tipo;
    double    num;
    char      texto[Q_VALOR];
} Filtro;

typedef struct {
    char   campo[Q_NOME];
    double alvo, tol, peso;
} Sim;

typedef struct {
    Filtro filtros[Q_MAX]; int n_filtros;
    Sim    sims[Q_MAX];    int n_sims;
    int    tem_accept;     double accept;
    char   ordem_campo[Q_NOME]; int ordem_desc;
    char   grupo[Q_MAX][Q_NOME]; int n_grupo;
    char   cruz[Q_MAX][Q_NOME];  int n_cruz;
} Query;

int query_carregar(const char *caminho, Query *q, char *err, size_t err_n);

int    query_filtros_ok(const Query *q, ITEM *it);

double query_similaridade(const Query *q, ITEM *it);

int    query_aceita(const Query *q, double sim);

int query_cmp_resultado(ITEM *a, ITEM *b, void *ctx);
int query_cmp_base(ITEM *a, ITEM *b, void *ctx);

#endif
