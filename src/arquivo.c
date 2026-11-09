#include <stdio.h>
#include <string.h>

#include "../headers/arquivo.h"
#include "../headers/item.h"
#include "../headers/query.h"


// Copia o cabeçalho do arquivo original para o arquivo de saída.
// A cópia é feita caractere por caractere para não depender de um tamanho fixo de buffer.
static bool copiar_cabecalho(FILE *entrada, FILE *saida) {

    int caractere;
    bool encontrou_caractere = false;

    while ((caractere = fgetc(entrada)) != EOF) {

        encontrou_caractere = true;

        if (fputc(caractere, saida) == EOF)
            return false;

        if (caractere == '\n')
            break;
    }

    if (ferror(entrada))
        return false;

    return encontrou_caractere;
}


// Carrega os registros de um arquivo para a estrutura escolhida.
bool arquivo_carregar(
    const char *caminho,
    Estrutura *base,
    size_t limite,
    size_t *quantidade,
    LEITOR_ITEM leitor,
    INSERIR_ITEM inserir,
    Query *query
) {

    if (caminho == NULL || base == NULL ||
        leitor == NULL || inserir == NULL)
        return false;

    if (quantidade != NULL)
        *quantidade = 0;

    FILE *arquivo = fopen(caminho, "r");

    if (arquivo == NULL) {
        perror("Erro ao abrir arquivo de entrada");
        return false;
    }

    // Ignora o cabeçalho do CSV.
    int caractere;

    while ((caractere = fgetc(arquivo)) != EOF) {
        if (caractere == '\n')
            break;
    }

    if (ferror(arquivo)) {
        fclose(arquivo);
        return false;
    }

    // Se chegou ao fim antes de encontrar o cabeçalho, o arquivo está vazio.
    if (caractere == EOF) {
        fclose(arquivo);
        return false;
    }

    size_t total = 0;
    bool sucesso = true;

    // Lê os registros enquanto houver dados e o limite não tiver sido atingido.
    // O módulo não conhece os campos do ITEM. Essa interpretação fica a cargo da função leitor.
    while (limite == 0 || total < limite) {

        ITEM *item = leitor(arquivo);

        // NULL indica fim do arquivo ou falha de leitura, conforme o contrato definido para o leitor.
        if (item == NULL) {
            if (ferror(arquivo))
                sucesso = false;

            break;
        }

        // Insere o ITEM na estrutura escolhida.
        if (!inserir(base, item, query)) {

            // Se a inserção falhou, a estrutura não assumiu a responsabilidade pelo ITEM.
            item_apagar(&item);

            sucesso = false;
            break;
        }

        total++;
    }

    if (quantidade != NULL)
        *quantidade = total;

    if (fclose(arquivo) != 0)
        sucesso = false;

    return sucesso;
}


// Estado compartilhado entre arquivo_escrever e o visitante que grava cada item.
typedef struct {
    FILE *saida;
    ESCRITOR_ITEM escritor;
    size_t limite;
    size_t total;
    bool erro;
} SAIDA_CTX;


// Visitante chamado pela estrutura a cada item, na ordem de acesso dela.
// Grava o item e interrompe o percurso quando o top-k e atingido ou quando ocorre erro.
static int escrever_visita(ITEM *item, void *ctx) {

    SAIDA_CTX *saida = (SAIDA_CTX *) ctx;

    // top_k = 0 significa sem limite.
    if (saida->limite != 0 && saida->total >= saida->limite)
        return 1;

    //O escritor específico do dataset transforma o ITEM em uma linha do arquivo.
    if (!saida->escritor(saida->saida, item)) {
        saida->erro = true;
        return 1;
    }

    saida->total++;

    return saida->limite != 0 && saida->total >= saida->limite;
}


// Escreve os resultados de uma busca em um arquivo CSV.
// O percurso é delegado à própria estrutura (estrutura_percorrer), então o mesmo código
// serve para qualquer tipo de estrutura, inclusive as que têm sublistas.
bool arquivo_escrever(
    const char *entrada,
    const char *saida,
    Estrutura *resultado,
    size_t top_k,
    size_t *quantidade,
    ESCRITOR_ITEM escritor
) {

    if (entrada == NULL || saida == NULL ||
        resultado == NULL || escritor == NULL)
        return false;

    // Evita apagar acidentalmente o arquivo original.
    if (strcmp(entrada, saida) == 0)
        return false;

    if (quantidade != NULL)
        *quantidade = 0;

    FILE *arq_entrada = fopen(entrada, "r");

    if (arq_entrada == NULL) {
        perror("Erro ao abrir arquivo original");
        return false;
    }

    FILE *arq_saida = fopen(saida, "w");

    if (arq_saida == NULL) {
        perror("Erro ao criar arquivo de saída");
        fclose(arq_entrada);
        return false;
    }

    bool sucesso = true;

    // Copia o cabeçalho original.
    if (!copiar_cabecalho(arq_entrada, arq_saida)) {
        sucesso = false;
    }

    SAIDA_CTX ctx = { arq_saida, escritor, top_k, 0, false };

    // Percorre a estrutura de resultados, gravando no máximo top_k itens.
    if (sucesso) {
        estrutura_percorrer(resultado, escrever_visita, &ctx);

        if (ctx.erro)
            sucesso = false;
    }

    if (ferror(arq_saida))
        sucesso = false;

    if (fclose(arq_entrada) != 0)
        sucesso = false;

    if (fclose(arq_saida) != 0)
        sucesso = false;

    if (quantidade != NULL)
        *quantidade = ctx.total;

    return sucesso;
}


bool arquivo_inserir_lista(Estrutura *e, ITEM *item, Query *q) {
    (void)q; return e && e->estrutura.geral && lista_inserir_atomo(e->estrutura.geral, item, -1);
}
bool arquivo_inserir_pilha(Estrutura *e, ITEM *item, Query *q) {
    (void)q; return e && e->estrutura.geral && pilha_empilhar(e->estrutura.geral, item);
}
bool arquivo_inserir_fila(Estrutura *e, ITEM *item, Query *q) {
    (void)q; return e && e->estrutura.geral && fila_inserir(e->estrutura.geral, item);
}
bool arquivo_inserir_seq(Estrutura *e, ITEM *item, Query *q) {
    (void)q; return e && e->estrutura.seq && lista_seq_inserir(e->estrutura.seq, item, -1);
}
bool arquivo_inserir_ordenado(Estrutura *e, ITEM *item, Query *q) {
    if (!e || !e->estrutura.seq || !q) return false;
    if (e->papel == PAPEL_RESULTADO)
        return lista_seq_inserir_ordenado_cmp(e->estrutura.seq, item, query_cmp_resultado, q);
    return lista_seq_inserir_ordenado_cmp(e->estrutura.seq, item, query_cmp_base, q);
}

bool arquivo_inserir_agrupado(Estrutura *e, ITEM *item, Query *q) {
    const char *campos[Q_MAX];
    if (!e || !e->estrutura.geral || !q || q->n_grupo < 1) return false;
    for (int i=0; i<q->n_grupo; i++) campos[i] = q->grupo[q->n_grupo-1-i];
    return lista_inserir_agrupado(e->estrutura.geral, item, campos, q->n_grupo);
}
bool arquivo_inserir_cruzado(Estrutura *e, ITEM *item, Query *q) {
    return e && e->estrutura.cruz && q && q->n_cruz == 2 &&
        lista_cruz_inserir(e->estrutura.cruz, item, q->cruz[0], q->cruz[1]);
}
