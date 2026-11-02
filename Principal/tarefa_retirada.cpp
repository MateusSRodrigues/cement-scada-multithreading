#include "tarefa_retirada.h"
#include "lista_encadeada.h"
#include <pthread.h>
#include <cstdio>

void* ThreadRetirada(void* arg) {
    ControleTarefa* controle = static_cast<ControleTarefa*>(arg);
    if (!controle) {
        LogConsolePrincipal("[RETIRADA_MSG] Erro: Ponteiro de controle nulo!\n");
        return nullptr;
    }

    ItemMensagem itemRecebido{};

    LogConsolePrincipal("[%s] Thread iniciada com sucesso.\n", controle->nomeTarefa);

    while (true) {
        // Aguarda e retira item da Lista 1 (verifica internamente bloqueio da tecla 'r' e término ESC)
        if (!RetirarMensagemLista1(&itemRecebido, controle)) {
            break; // ESC acionado
        }

        // Exibe o conteudo retirado na console principal (requisito de certificacao da Etapa 1)
        if (itemRecebido.tipo == TIPO_DADOS_PROCESSO) {
            LogConsolePrincipal("[RETIRADA - PROC] %s\n", itemRecebido.texto);
        }
        else if (itemRecebido.tipo == TIPO_ESCALONAMENTO_PCP) {
            LogConsolePrincipal("[RETIRADA - PCP ] %s\n", itemRecebido.texto);
        }
    }

    LogConsolePrincipal("[%s] Thread finalizada de forma limpa.\n", controle->nomeTarefa);
    return nullptr;
}