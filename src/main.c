#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "../headers/item.h"
#include "../headers/lista.h"
#include "../headers/arquivo.h"
#include "../headers/query.h"

static double agora_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

static void uso(const char *prog)
{
    fprintf(stderr,
        "Uso: %s --reader R --input ARQ --base ESTR --result ESTR --query ARQ\n"
        "          --output ARQ [--top-k N] [--limit N]\n"
        "  R:    nasa | ancine\n"
        "  ESTR: pilha fila lista_seq lista_enc lista_cabeca lista_ord lista_gen lista_cruz\n"
        "  --top-k 0 e --limit 0 significam 'todos'.\n", prog);
}

typedef struct {
    const char    *nome;
    LEITOR_ITEM    ler;
    ESCRITOR_ITEM  escrever;
} Dataset;

static const Dataset DATASETS[] = {
    { "nasa",   item_ler_nasa,   item_escrever_nasa   },
    { "ancine", item_ler_ancine, item_escrever_ancine },
    { "cinema", item_ler_ancine, item_escrever_ancine },
};

typedef enum { PAPEL_BASE, PAPEL_RESULTADO } Papel;

typedef struct {
    const char  *nome;

    LISTA     *(*criar)(const Query *q, Papel papel);
    INSERIR_ITEM inserir;
} Estrutura;

static LISTA *criar_geral(const Query *q, Papel papel)
{
    (void)q; (void)papel;
    return lista_criar();
}

static const Estrutura ESTRUTURAS[] = {
    { "pilha",        .geral = criar_geral, arquivo_inserir_pilha },
    { "fila",         .geral = criar_geral, arquivo_inserir_fila  },
    // lista_enc precisa de criar/inserir proprios se for diferenciar de lista_cabeca
    { "lista_enc",    .geral = criar_geral, arquivo_inserir_lista },
    { "lista_cabeca", .geral = criar_geral, arquivo_inserir_lista },

    // preencher criar e inserir com o que ta faltando
    // lista_ord: comparador = query_cmp_base (base) ou query_cmp_resultado (resultado), ctx = q
    // lista_gen: niveis = q->grupo[0..n_grupo-1] | lista_cruz: organizacoes = q->cruz[0..n_cruz-1]
    { "lista_seq",    NULL, NULL },
    { "lista_ord",    NULL, NULL },
    { "lista_gen",    criar_geral, NULL },
    { "lista_cruz",   NULL, NULL },
};

#define N_ELEM(v) (sizeof (v) / sizeof (v)[0])

static const Dataset *achar_dataset(const char *nome)
{
    size_t i;
    for (i = 0; i < N_ELEM(DATASETS); i++)
        if (!strcmp(DATASETS[i].nome, nome)) return &DATASETS[i];
    return NULL;
}

static const Estrutura *achar_estrutura(const char *nome)
{
    size_t i;
    for (i = 0; i < N_ELEM(ESTRUTURAS); i++)
        if (!strcmp(ESTRUTURAS[i].nome, nome)) return &ESTRUTURAS[i];
    return NULL;
}

typedef struct {
    const char *reader, *input, *base, *result, *query, *output;
    long top_k, limit;
} Args;

static int parse_long(const char *s, long *out)
{
    char *fim;
    errno = 0;
    *out = strtol(s, &fim, 10);
    return errno == 0 && *s != '\0' && *fim == '\0' && *out >= 0;
}

static int parse_args(int argc, char **argv, Args *a)
{
    int i;
    memset(a, 0, sizeof *a);
    for (i = 1; i < argc; i++) {
        char *chave = argv[i], *valor = strchr(chave, '=');
        if (strncmp(chave, "--", 2) != 0) {
            fprintf(stderr, "argumento inesperado: %s\n", chave);
            return -1;
        }
        chave += 2;
        if (valor) { *valor++ = '\0'; }
        else if (i + 1 < argc) { valor = argv[++i]; }
        else { fprintf(stderr, "falta valor para --%s\n", chave); return -1; }

        if      (!strcmp(chave, "reader")) a->reader = valor;
        else if (!strcmp(chave, "input"))  a->input  = valor;
        else if (!strcmp(chave, "base"))   a->base   = valor;
        else if (!strcmp(chave, "result")) a->result = valor;
        else if (!strcmp(chave, "query"))  a->query  = valor;
        else if (!strcmp(chave, "output")) a->output = valor;
        else if (!strcmp(chave, "top-k") || !strcmp(chave, "top_k")) {
            if (!parse_long(valor, &a->top_k)) {
                fprintf(stderr, "--top-k invalido: %s\n", valor);
                return -1;
            }
        } else if (!strcmp(chave, "limit")) {
            if (!parse_long(valor, &a->limit)) { fprintf(stderr, "--limit invalido: %s\n", valor); return -1;
            }
        } else { fprintf(stderr, "opcao desconhecida: --%s\n", chave); return -1; }
    }
    if (!a->reader || !a->input || !a->base || !a->result || !a->query || !a->output) {
        fprintf(stderr, "faltam argumentos obrigatorios\n");
        return -1;
    }
    return 0;
}

static const Estrutura *validar_estrutura(const char *opcao, const char *nome, const Query *q)
{
    const Estrutura *e = achar_estrutura(nome);
    if (!e) { fprintf(stderr, "%s: estrutura desconhecida '%s'\n", opcao, nome); return NULL; }
    if (!e->criar) { fprintf(stderr, "%s: '%s' ainda nao foi implementada\n", opcao, nome); return NULL; }
    if (!strcmp(nome, "lista_gen") && q->n_grupo < 1) {
        fprintf(stderr, "%s=lista_gen exige o comando GROUP na consulta\n", opcao); return NULL;
    }
    if (!strcmp(nome, "lista_cruz") && q->n_cruz < 2) {
        fprintf(stderr, "%s=lista_cruz exige CROSS com pelo menos 2 campos\n", opcao); return NULL;
    }
    return e;
}

typedef struct {
    const Query *q;
    LISTA       *res;
    INSERIR_ITEM inserir;
    size_t       vistos, aceitos;
    double       insert_ms;
    int          erro;
} BuscaCtx;

static int visita_busca(ITEM *it, void *p)
{
    BuscaCtx *c = p;
    double sim, t0;

    c->vistos++;
    if (!query_filtros_ok(c->q, it)) return 0;
    sim = query_similaridade(c->q, it);
    if (!query_aceita(c->q, sim)) return 0;

    item_set_sim(it, sim);
    t0 = agora_ms();
    if (!c->inserir(c->res, it)) {
        c->erro = 1;
        return 1;
    }
    c->insert_ms += agora_ms() - t0;
    c->aceitos++;
    return 0;
}

int main(int argc, char **argv)
{
    Args a;
    Query q;
    char err[256] = "";
    const Dataset *ds;
    const Estrutura *eb, *er;
    LISTA *base = NULL, *res = NULL;
    size_t input_records = 0, output_records = 0;
    double t0, load_ms = 0, search_ms = 0, write_ms = 0;
    BuscaCtx bc;
    int status = 1;

    if (argc < 2 || !strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")) {
        uso(argv[0]);
        return 1;
    }
    if (parse_args(argc, argv, &a) != 0) {
        uso(argv[0]);
        printf("STATUS=ERROR\n"); return 1;
    }

    ds = achar_dataset(a.reader);
    if (!ds) {
        fprintf(stderr, "reader desconhecido: '%s'\n", a.reader);
        goto fim;
    }
    if (query_carregar(a.query, &q, err, sizeof err) != 0) {
        fprintf(stderr, "%s\n", err);
        goto fim;
    }

    eb = validar_estrutura("--base", a.base, &q);
    er = validar_estrutura("--result", a.result, &q);
    if (!eb || !er) goto fim;

    base = eb->criar(&q, PAPEL_BASE);
    res  = er->criar(&q, PAPEL_RESULTADO);
    if (!base || !res) {
        fprintf(stderr, "falha ao criar estruturas\n");
        goto fim;
    }

    t0 = agora_ms();
    if (!arquivo_carregar(a.input, base, (size_t)a.limit, &input_records, ds->ler, eb->inserir)) {
        fprintf(stderr, "erro ao carregar '%s' (lidos %zu registros)\n", a.input, input_records);
        goto fim;
    }
    load_ms = agora_ms() - t0;

    memset(&bc, 0, sizeof bc);
    bc.q = &q; bc.res = res; bc.inserir = er->inserir;
    t0 = agora_ms();
    lista_percorrer(base, visita_busca, &bc);
    search_ms = (agora_ms() - t0) - bc.insert_ms;
    if (bc.erro) {
        fprintf(stderr, "falha ao inserir no resultado\n");
        goto fim;
    }

    t0 = agora_ms();
    if (!arquivo_escrever(a.input, a.output, res, (size_t)a.top_k, &output_records, ds->escrever)) {
        fprintf(stderr, "erro ao escrever '%s'\n", a.output);
        goto fim;
    }
    write_ms = agora_ms() - t0;

    printf("STATUS=OK\n");
    printf("INPUT_RECORDS=%zu\n", input_records);
    printf("MATCHED_RECORDS=%zu\n", bc.aceitos);
    printf("OUTPUT_RECORDS=%zu\n", output_records);
    printf("LOAD_MS=%.3f\n", load_ms);
    printf("SEARCH_MS=%.3f\n", search_ms);
    printf("RESULT_INSERT_MS=%.3f\n", bc.insert_ms);
    printf("WRITE_MS=%.3f\n", write_ms);
    status = 0;

fim:
    if (status != 0) printf("STATUS=ERROR\n");
    if (res)  lista_liberar_nos(&res);
    if (base) lista_apagar(&base);
    return status;
}
