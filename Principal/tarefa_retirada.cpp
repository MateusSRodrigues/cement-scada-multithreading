#include "common.h"
#include <pthread.h>

// Importação da função de consumo implementada no lista_encadeada.cpp
extern bool RetirarMensagemLista1(ItemMensagem* itemDestino, HANDLE hEvTerm);

// Assinatura padrão exigida pela biblioteca Pthreads
void* TarefaRetirada(void* arg) {
    // Cast obrigatório em C++ para aceder aos handles da tarefa
    ControleTarefa* controle = static_cast<ControleTarefa*>(arg);
    ItemMensagem itemRecebido;

    printf("[%s] Iniciada.\n", controle->nomeTarefa);

    while (true) {
        // 1. Verifica bloqueio da tecla 'r' e encerramento geral (ESC)
        if (!AguardarPermissaoExecucaoOuTermino(controle->hEvRun, controle->hEvTerm, controle->nomeTarefa)) {
            break; // Sai imediatamente do ciclo se ESC for premido
        }

        // 2. Aguarda e retira item (bloqueia automaticamente se a lista estiver vazia)
        if (!RetirarMensagemLista1(&itemRecebido, controle->hEvTerm)) {
            break; // ESC premido enquanto aguardava por mensagens
        }

        // 3. Exibe o conteúdo retirado (Etapa 1: saída no ecrã principal)
        if (itemRecebido.tipo == TIPO_DADOS_PROCESSO) {
            printf("[RETIRADA - PROC] %s\n", itemRecebido.texto);
        }
        else if (itemRecebido.tipo == TIPO_ESCALONAMENTO_PCP) {
            printf("[RETIRADA - PCP ] %s\n", itemRecebido.texto);
        }
    }

    printf("[%s] Encerrando thread...\n", controle->nomeTarefa);
    return nullptr;
}