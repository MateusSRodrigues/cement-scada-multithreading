#pragma once
#ifndef LISTA_ENCADEADA_H
#define LISTA_ENCADEADA_H

#include "../include/common.h"

// Inicializa os semaforos e os mutexes da lista e do console
void InicializarLista1();

// Esvazia a lista e destroi os objetos de sincronismo
void DestruirLista1();

// Imprime mensagens no console principal com exclusao mutua via Pthreads
void LogConsolePrincipal(const char* formato, ...);

// Verifica bloqueio (hEvRun) ou encerramento (hEvTerm) usando log thread-safe
bool AguardarPermissaoThreadPrincipal(ControleTarefa* ctrl);

// Insere item na lista respeitando bloqueio e encerramento. Retorna false se ESC for acionado.
bool DepositarMensagemLista1(const ItemMensagem& item, ControleTarefa* ctrl);

// Remove item da lista respeitando bloqueio e encerramento. Retorna false se ESC for acionado.
bool RetirarMensagemLista1(ItemMensagem* itemDestino, ControleTarefa* ctrl);

#endif // LISTA_ENCADEADA_H