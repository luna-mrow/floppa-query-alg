#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>
#include "../headers/query.h"

static char *trim(char *s)
{
    char *fim;
    while (isspace((unsigned char)*s)) s++;
    fim = s + strlen(s);
    while (fim > s && isspace((unsigned char)fim[-1])) *--fim = '\0';
    return s;
}

static int dividir(char *linha, char *tok[], int max)
{
    int n = 0;
    char *p = linha;
    while (n < max) {
        char *barra = strchr(p, '|');
        if (barra) *barra = '\0';
        tok[n++] = trim(p);
        if (!barra) break;
        p = barra + 1;
    }
    return n;
}

static int numero(const char *s, double *out)
{
    char *fim;
    if (*s == '\0') return 0;
    *out = strtod(s, &fim);
    return *fim == '\0';
}

static int copiar_nome(char *dst, const char *src)
{
    if (*src == '\0' || strlen(src) >= Q_NOME) return -1;
    strcpy(dst, src);
    return 0;
}

static int falha(char *err, size_t n, int linha, const char *msg)
{
    snprintf(err, n, "consulta, linha %d: %s", linha, msg);
    return -1;
}

static int parse_filtro(Query *q, char *tok[], int nt, int ln, char *err, size_t en)
{
    Filtro *f;
    char *fim;
    if (nt != 4) return falha(err, en, ln, "FILTER espera FILTER|campo|op|valor");
    if (q->n_filtros >= Q_MAX) return falha(err, en, ln, "FILTER demais");
    f = &q->filtros[q->n_filtros];
    if (copiar_nome(f->campo, tok[1]) != 0) return falha(err, en, ln, "campo invalido");
    if      (!strcmp(tok[2], "==") || !strcmp(tok[2], "=")) f->op = OP_EQ;
    else if (!strcmp(tok[2], "!="))  f->op = OP_NE;
    else if (!strcmp(tok[2], "<"))   f->op = OP_LT;
    else if (!strcmp(tok[2], "<="))  f->op = OP_LE;
    else if (!strcmp(tok[2], ">"))   f->op = OP_GT;
    else if (!strcmp(tok[2], ">="))  f->op = OP_GE;
    else return falha(err, en, ln, "operador invalido (use == != < <= > >=)");
    if (strlen(tok[3]) >= Q_VALOR) return falha(err, en, ln, "valor muito longo");
    strcpy(f->texto, tok[3]);
    f->num = strtod(f->texto, &fim);
    if (f->texto[0] != '\0' && *fim == '\0') f->tipo = VAL_NUM;
    else if (!strcasecmp(f->texto, "true"))  { f->tipo = VAL_BOOL; f->num = 1; }
    else if (!strcasecmp(f->texto, "false")) { f->tipo = VAL_BOOL; f->num = 0; }
    else f->tipo = VAL_STR;
    q->n_filtros++;
    return 0;
}

static int parse_sim(Query *q, char *tok[], int nt, int ln, char *err, size_t en)
{
    Sim *s;
    if (nt != 5) return falha(err, en, ln, "SIM espera SIM|campo|alvo|tolerancia|peso");
    if (q->n_sims >= Q_MAX) return falha(err, en, ln, "SIM demais");
    s = &q->sims[q->n_sims];
    if (copiar_nome(s->campo, tok[1]) != 0) return falha(err, en, ln, "campo invalido");
    if (!numero(tok[2], &s->alvo) || !numero(tok[3], &s->tol) || !numero(tok[4], &s->peso))
        return falha(err, en, ln, "alvo, tolerancia e peso devem ser numeros");
    if (s->tol <= 0)  return falha(err, en, ln, "tolerancia deve ser > 0");
    if (s->peso <= 0) return falha(err, en, ln, "peso deve ser > 0");
    q->n_sims++;
    return 0;
}

static int parse_lista(char dst[][Q_NOME], int *n, char *tok[], int nt, int nt_max, int ln, char *err, size_t en)
{
    int i;
    double aux;
    if (nt < 2 || nt - 1 > nt_max) return falha(err, en, ln, "lista de campos invalida");
    for (i = 1; i < nt; i++) {
        if (copiar_nome(dst[i - 1], tok[i]) != 0) return falha(err, en, ln, "campo invalido");
        if (!numero(tok[i], &aux)) return falha(err, en, ln, "campos cruzados/agrupados devem ser numeros");
    }
    *n = nt - 1;
    return 0;
}

int query_carregar(const char *caminho, Query *q, char *err, size_t err_n)
{
    FILE *f = fopen(caminho, "r");
    char *linha = NULL;
    size_t cap = 0;
    int ln = 0, rc = 0;

    memset(q, 0, sizeof *q);
    strcpy(q->ordem_campo, "similarity");
    q->ordem_desc = 1;

    if (!f) {
        snprintf(err, err_n, "nao foi possivel abrir a consulta '%s'", caminho);
        return -1;
    }

    while (rc == 0 && getline(&linha, &cap, f) != -1) {
        char *tok[8], *l = trim(linha);
        int nt;
        ln++;
        if (*l == '\0' || *l == '#') continue;
        nt = dividir(l, tok, 8);
        if      (!strcasecmp(tok[0], "FILTER")) rc = parse_filtro(q, tok, nt, ln, err, err_n);
        else if (!strcasecmp(tok[0], "SIM"))    rc = parse_sim(q, tok, nt, ln, err, err_n);
        else if (!strcasecmp(tok[0], "ACCEPT")) {
            if (nt != 2 || !numero(tok[1], &q->accept)) rc = falha(err, err_n, ln, "ACCEPT espera um numero");
            else q->tem_accept = 1;
        } else if (!strcasecmp(tok[0], "ORDER")) {
            if ((nt != 2 && nt != 3) || copiar_nome(q->ordem_campo, tok[1]) != 0)
                rc = falha(err, err_n, ln, "ORDER espera ORDER|campo|ASC/DESC");
            else if (nt == 3 && !strcasecmp(tok[2], "DESC")) q->ordem_desc = 1;
            else if (nt == 3 && !strcasecmp(tok[2], "ASC"))  q->ordem_desc = 0;
            else if (nt == 3) rc = falha(err, err_n, ln, "direcao deve ser ASC ou DESC");
            else q->ordem_desc = 0;
        } else if (!strcasecmp(tok[0], "GROUP")) rc = parse_lista(q->grupo, &q->n_grupo, tok, nt, Q_MAX, ln, err, err_n);
        else if (!strcasecmp(tok[0], "CROSS"))   rc = parse_lista(q->cruz, &q->n_cruz, tok, nt, 2, ln, err, err_n);
        else rc = falha(err, err_n, ln, "comando desconhecido");
    }
    free(linha);
    fclose(f);
    return rc;
}

static int aplica_op(Operador op, int c)
{
    switch (op) {
    case OP_EQ: return c == 0;
    case OP_NE: return c != 0;
    case OP_LT: return c < 0;
    case OP_LE: return c <= 0;
    case OP_GT: return c > 0;
    default:    return c >= 0;
    }
}

static int filtro_ok(const Filtro *f, ITEM *it)
{
    double x;
    const char *s;
    if (f->tipo == VAL_NUM) {
        if (!item_get_num(it, f->campo, &x)) return 0;
        return aplica_op(f->op, (x > f->num) - (x < f->num));
    }
    if (f->tipo == VAL_BOOL && item_get_num(it, f->campo, &x))
        return aplica_op(f->op, (x > f->num) - (x < f->num));
    s = item_get_str(it, f->campo);
    if (!s) return 0;
    while (isspace((unsigned char)*s)) s++;
    return aplica_op(f->op, strcasecmp(s, f->texto));
}

int query_filtros_ok(const Query *q, ITEM *it)
{
    int i;
    for (i = 0; i < q->n_filtros; i++)
        if (!filtro_ok(&q->filtros[i], it)) return 0;
    return 1;
}

double query_similaridade(const Query *q, ITEM *it)
{
    double soma = 0, pesos = 0, x;
    int i;
    if (q->n_sims == 0) return 1.0;
    for (i = 0; i < q->n_sims; i++) {
        const Sim *s = &q->sims[i];
        double v = 0;
        if (item_get_num(it, s->campo, &x)) {
            v = 1.0 - fabs(x - s->alvo) / s->tol;
            if (v < 0) v = 0;
        }
        soma += s->peso * v;
        pesos += s->peso;
    }
    return soma / pesos;
}

int query_aceita(const Query *q, double sim)
{
    if (q->n_sims == 0 || !q->tem_accept) return 1;
    return sim + 1e-12 >= q->accept;
}

static int cmp_campo(ITEM *a, ITEM *b, const char *campo, int desc)
{
    double x = 0, y = 0;
    int ok_a, ok_b, c;
    if (!strcasecmp(campo, "similarity")) {
        x = item_get_sim(a); y = item_get_sim(b);
        c = (x > y) - (x < y);
        return desc ? -c : c;
    }
    ok_a = item_get_num(a, campo, &x);
    ok_b = item_get_num(b, campo, &y);
    if (ok_a && ok_b) {
        c = (x > y) - (x < y);
    } else if (!ok_a && !ok_b) {
        const char *sa = item_get_str(a, campo), *sb = item_get_str(b, campo);
        if (!sa && !sb) return 0;
        if (!sa) return 1;
        if (!sb) return -1;
        c = strcasecmp(sa, sb);
    } else {
        return ok_a ? -1 : 1;
    }
    return desc ? -c : c;
}

int query_cmp_resultado(ITEM *a, ITEM *b, void *ctx)
{
    const Query *q = ctx;
    return cmp_campo(a, b, q->ordem_campo, q->ordem_desc);
}

int query_cmp_base(ITEM *a, ITEM *b, void *ctx)
{
    const Query *q = ctx;
    if (strcasecmp(q->ordem_campo, "similarity") != 0)
        return cmp_campo(a, b, q->ordem_campo, q->ordem_desc);
    if (q->n_sims > 0)
        return cmp_campo(a, b, q->sims[0].campo, 0);
    return 0;
}
