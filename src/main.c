#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "../headers/arquivo.h"

static double agora_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

static long contar_linhas(const char *nome_arquivo) 
{
    FILE* fptr = fopen(nome_arquivo, "r");
    int ch; long cont = 0;

    while ((ch = fgetc(fptr)) != EOF) {
        if (ch == '\n') cont++;
    }

    fclose(fptr);
    return cont;
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

static const Dataset DATASETS[] = {
    { "nasa", item_ler_nasa, item_escrever_nasa },
    { "ancine", item_ler_ancine, item_escrever_ancine },
    { "cinema", item_ler_ancine, item_escrever_ancine },
};

static LISTA *criar_geral(const Args *a, Papel papel)
{
    (void)a; (void)papel;
    return lista_criar();
}

static LISTA_SEQ *criar_seq(const Args *a, Papel papel)
{
    long size;
    switch (papel) {
        case PAPEL_BASE:
            if (a->limit == 0) size = contar_linhas(a->input);
            else size = a->limit;
            break;
        case PAPEL_RESULTADO:
            if (a->top_k == 0) size = a->limit != 0 ? a->limit : contar_linhas(a->input);
            else size = a->top_k;
    }
    return lista_seq_criar((int) size);
}

static LISTA_CRUZ *criar_cruz(const Args *a, Papel papel)
{
    (void)a; (void)papel;
    return lista_cruz_criar();
}

static bool estrutura_criar(Estrutura *e, const Args *a, Papel papel) 
{
    switch (e->tipo) {
        case GERAL:
            e->estrutura.geral = criar_geral(a, papel);
            break;
        case SEQ:
            e->estrutura.seq = criar_seq(a, papel);
            break;
        case CRUZ:
            e->estrutura.cruz = criar_cruz(a, papel);
            break;
        default: return false;
    }
    return true;
}

static bool estrutura_apagar(Estrutura *e) 
{
    switch (e->tipo) {
        case GERAL:
            lista_apagar(&e->estrutura.geral);
            break;
        case SEQ:
            lista_seq_apagar(&e->estrutura.seq);
            break;
        case CRUZ:
            lista_cruz_apagar(&e->estrutura.cruz);
            break;
        default: return false;
    }
    return true;
}

static Estrutura ESTRUTURAS[] = {
    { "pilha", GERAL, { NULL }, arquivo_inserir_pilha },
    { "fila", GERAL, { NULL }, arquivo_inserir_fila },
    { "lista_enc", GERAL, { NULL }, arquivo_inserir_lista },
    { "lista_cabeca", GERAL, { NULL }, arquivo_inserir_lista },
    { "lista_gen", GERAL, { NULL }, arquivo_inserir_agrupado },
    { "lista_seq", SEQ, { NULL }, arquivo_inserir_seq },
    { "lista_ord", SEQ, { NULL }, arquivo_inserir_ordenado },
    { "lista_cruz", CRUZ, { NULL }, arquivo_inserir_cruzado },
};

#define N_ELEM(v) (sizeof (v) / sizeof (v)[0])

static const Dataset *achar_dataset(const char *nome)
{
    size_t i;
    for (i = 0; i < N_ELEM(DATASETS); i++)
        if (!strcmp(DATASETS[i].nome, nome)) return &DATASETS[i];
    return NULL;
}

static Estrutura *achar_estrutura(const char *nome)
{
    size_t i;
    for (i = 0; i < N_ELEM(ESTRUTURAS); i++)
        if (!strcmp(ESTRUTURAS[i].nome, nome)) return &ESTRUTURAS[i];
    return NULL;
}

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

static Estrutura *validar_estrutura(const char *opcao, const char *nome, const Query *q)
{
    Estrutura *e = achar_estrutura(nome);
    if (!e) {
        fprintf(stderr, "%s: estrutura desconhecida '%s'\n", opcao, nome);
        return NULL;
    }
    if (!strcmp(nome, "lista_gen") && q->n_grupo < 1) {
        fprintf(stderr, "%s=lista_gen exige o comando GROUP na consulta\n", opcao);
        return NULL;
    }
    if (!strcmp(nome, "lista_cruz") && q->n_cruz != 2) {
        fprintf(stderr, "%s=lista_cruz exige CROSS com 2 campos\n", opcao);
        return NULL;
    }
    return e;
}

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
    if (!c->res->inserir(c->res, it, c->q)) {
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
    Estrutura *base, *res;
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

    base = validar_estrutura("--base", a.base, &q);
    res = validar_estrutura("--result", a.result, &q);
    if (!base || !res) goto fim;

    estrutura_criar(base, &a, PAPEL_BASE);
    estrutura_criar(res, &a, PAPEL_RESULTADO);
    if (!base || !res) {
        fprintf(stderr, "falha ao criar estruturas\n");
        goto fim;
    }

    t0 = agora_ms();
    if (!arquivo_carregar(a.input, base, (size_t)a.limit, &input_records, ds->ler, &q)) {
        fprintf(stderr, "erro ao carregar '%s' (lidos %zu registros)\n", a.input, input_records);
        goto fim;
    }
    load_ms = agora_ms() - t0;

    memset(&bc, 0, sizeof bc);
    bc.q = &q; bc.res = res;
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
    if (res)  estrutura_apagar(res);
    if (base) estrutura_apagar(base);
    return status;
}
