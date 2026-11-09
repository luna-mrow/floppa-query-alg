#include <stddef.h>

#include "../headers/def.h"

int estrutura_percorrer(const Estrutura *e, VISITA_ITEM visita, void *ctx) {
  if (e == NULL || visita == NULL) return 0;

  switch (e->tipo) {
    case GERAL: return lista_percorrer(e->estrutura.geral, visita, ctx);
    case SEQ:   return lista_seq_percorrer(e->estrutura.seq, visita, ctx);
    case CRUZ:  return lista_cruz_percorrer(e->estrutura.cruz, visita, ctx);
  }

  return 0;
}
