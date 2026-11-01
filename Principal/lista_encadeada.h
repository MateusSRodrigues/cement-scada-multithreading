#pragma once
#ifndef LISTA_ENCADEADA_H
#define LISTA_ENCADEADA_H

#include "../include/common.h"

// Inicializa os semaforos e o mutex da lista
void InicializarLista1();

// Esvazia a lista e destroi os objetos de sincronismo
void DestruirLista1();

// Insere item na lista. Retorna false se ESC for premido durante a espera.
bool DepositarMensagemLista1(ItemMensagem item, HANDLE hEvTerm, const char* nomeProdutor);

// Remove item da lista. Retorna false se ESC for premido durante a espera.
bool RetirarMensagemLista1(ItemMensagem* itemDestino, HANDLE hEvTerm);

#endif // LISTA_ENCADEADA_H