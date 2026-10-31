#ifndef LISTA_CIRCULAR1_H
#define LISTA_CIRCULAR1_H

#include "common.h"

void InicializarLista1(void);
void DestruirLista1(void);

// Retorna true se inseriu, false se recebeu comando ESC enquanto aguardava vaga
bool DepositarMensagemLista1(ItemMensagem item, HANDLE hEvTerm, const char* nomeProdutor);
bool RetirarMensagemLista1(ItemMensagem* itemDestino, HANDLE hEvTerm);

#endif