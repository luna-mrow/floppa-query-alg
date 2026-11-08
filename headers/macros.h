#include "lista.h"
#include "lista_seq.h"
#include "lista_cruz.h"
#include "query.h"

typedef struct estrutura_ Estrutura;

// Modificar conforme item.c
typedef ITEM* (*LEITOR_ITEM)(FILE *arquivo);

// Modificar conforme item.c
typedef bool (*ESCRITOR_ITEM)(FILE *arquivo, ITEM *item);

// Modificar conforme item.c
typedef bool (*INSERIR_ITEM)(Estrutura *estrutura, ITEM *item, Query *query);

typedef struct {
    const char    *nome;
    LEITOR_ITEM    ler;
    ESCRITOR_ITEM  escrever;
} Dataset;

typedef enum { PAPEL_BASE, PAPEL_RESULTADO } Papel;
typedef enum { GERAL, SEQ, CRUZ } TipoEstrutura;

typedef struct {
    const char *reader, *input, *base, *result, *query, *output;
    long top_k, limit;
} Args;

typedef struct estrutura_ {
    const char  *nome;
    TipoEstrutura tipo;
    union {
        LISTA *geral;
        LISTA_SEQ *seq;
        LISTA_CRUZ *cruz;
    } estrutura;

    union {
        LISTA *(*geral)(const Args *a, const Query *q, Papel papel);
        LISTA_SEQ *(*seq)(const Args *a, const Query *q, Papel papel);
        LISTA_CRUZ *(*cruz)(const Args *a, const Query *q, Papel papel);
    } criar;
    INSERIR_ITEM inserir;
} Estrutura;

typedef struct {
    const Query *q;
    Estrutura *res;
    size_t       vistos, aceitos;
    double       insert_ms;
    int          erro;
} BuscaCtx;
