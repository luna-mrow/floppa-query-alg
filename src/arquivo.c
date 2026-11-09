#include <stdio.h>
#include <string.h>

#include "../headers/arquivo.h"
#include "../headers/item.h"


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
    Query *query
) {

    if (caminho == NULL || base == NULL ||
        leitor == NULL)
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
        if (!base->inserir(base, item, query)) {

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


// Escreve os resultados de uma busca em um arquivo CSV.
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
    size_t total = 0;

    // Copia o cabeçalho original.
    if (!copiar_cabecalho(arq_entrada, arq_saida)) {
        sucesso = false;
    }

    // Percorre a estrutura de resultados.
    // Esta implementação exporta listas planas, contendo apenas ITEMs.
    for (int pos = 0; sucesso && (top_k == 0 || total < top_k); pos++) {

        // Não existe elemento na posição consultada.
        if (lista_get_tipo(resultado, pos) == -1)
            break;

        ITEM *item = lista_get_atomo(resultado, pos);

        // Um NULL nessa posição pode indicar uma sublista.
        // A exportação de estruturas aninhadas será tratada separadamente.
        if (item == NULL) {
            sucesso = false;
            break;
        }

        //O escritor específico do dataset transforma o ITEM em uma linha do arquivo.
        if (!escritor(arq_saida, item)) {
            sucesso = false;
            break;
        }

        total++;
    }

    if (ferror(arq_saida))
        sucesso = false;

    if (fclose(arq_entrada) != 0)
        sucesso = false;

    if (fclose(arq_saida) != 0)
        sucesso = false;

    if (quantidade != NULL)
        *quantidade = total;

    return sucesso;
}


bool arquivo_inserir_lista(Estrutura *estrutura, ITEM *item, Query *query) {
    (void) query;
    return lista_inserir_atomo(estrutura->estrutura.geral, item, -1);
}

bool arquivo_inserir_pilha(Estrutura *estrutura, ITEM *item, Query *query) {
    (void) query;
    return pilha_empilhar(estrutura->estrutura.geral, item);
}

bool arquivo_inserir_fila(Estrutura *estrutura, ITEM *item, Query *query) {
    (void) query;
    return fila_inserir(estrutura->estrutura.geral, item);
}

bool arquivo_inserir_seq(Estrutura *estrutura, ITEM *item, Query *query) {
    (void) query;
    return lista_seq_inserir(estrutura->estrutura.seq, item, -1);
}

bool arquivo_inserir_ordenado(Estrutura *estrutura, ITEM *item, Query *query) {
    return lista_seq_inserir_ordenado(estrutura->estrutura.seq, item, query->ordem_campo, query->ordem_desc);
}

bool arquivo_inserir_agrupado(Estrutura *estrutura, ITEM *item, Query *query) {
    return lista_inserir_agrupado(estrutura->estrutura.geral, item, query->grupo, query->n_grupo);
}

bool arquivo_inserir_cruzado(Estrutura *estrutura, ITEM *item, Query *query) {
    return lista_inserir_cruzado(estrutura->estrutura.cruz, item, query->cruz[0], query->cruz[1]);
}
