#include "lista_circular1.h"
#include <cstdio>

// Funções vazias para enganar o compilador
void InicializarLista1(void) {
    std::printf("[MOCK LISTA 1] Lista virtual inicializada para testes.\n");
}

void DestruirLista1(void) {
}

// A função que o seu Supervisório e PCP vão chamar
bool DepositarMensagemLista1(ItemMensagem item, HANDLE hEvTerm, const char* nomeProdutor) {
    // Ao invés de usar semáforos, apenas imprime a string na tela
    std::printf("[%s -> MOCK LISTA] String gerada: %s\n", nomeProdutor, item.texto);
    
    // Retorna 'true' simulando que inseriu com sucesso
    return true; 
}

// Função que a Pessoa B vai usar futuramente (você não usa)
bool RetirarMensagemLista1(ItemMensagem* itemDestino, HANDLE hEvTerm) {
    return false;
}