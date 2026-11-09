#ifndef ITEM_H
#define ITEM_H

#include <stdio.h>
#include <stdbool.h>

typedef struct item_ ITEM;

//funcao chamada pelas estruturas a cada item visitado em um percurso
//retorna 0 para continuar e qualquer outro valor para interromper o percurso
typedef int (*VISITA_ITEM)(ITEM *item, void *ctx);

ITEM* item_copiar(ITEM*);
void item_apagar(ITEM**);

int item_get_id(ITEM*);

int item_get_num(ITEM*, const char *campo, double *out);
const char *item_get_str(ITEM*, const char *campo);

void item_set_sim(ITEM*, double sim);
double item_get_sim(ITEM*);

ITEM* item_ler_nasa(FILE*);
bool item_escrever_nasa(FILE*, ITEM*);

ITEM* item_ler_ancine(FILE*);
bool item_escrever_ancine(FILE*, ITEM*);

#endif
