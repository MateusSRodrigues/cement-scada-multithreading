#include "tarefa_pcp.h"
#include "lista_encadeada.h"
#include <random>
#include <cstdio>
#include <cstring>

// Gerador de numeros inteiros pseudoaleatorios com semente dinamica
static int GerarInteiroAleatorio(int minVal, int maxVal) {
    static thread_local std::mt19937 gerador((std::random_device{}() ^ GetCurrentThreadId()) + 0x9E3779B9u);
    std::uniform_int_distribution<int> dist(minVal, maxVal);
    return dist(gerador);
}

void* ThreadPCP(void* arg) {
    ControleTarefa* ctrl = static_cast<ControleTarefa*>(arg);
    if (!ctrl) {
        LogConsolePrincipal("[PCP] Erro: Ponteiro de controle nulo!\n");
        return nullptr;
    }

    int nseq = NSEQ_MIN; // Inicia a contagem sequencial em 1
    char horaPrevista[16] = { 0 };
    char op[16] = { 0 };
    ItemMensagem item{};
    item.tipo = TIPO_ESCALONAMENTO_PCP; // Sempre tipo 2

    LogConsolePrincipal("[%s] Thread iniciada com sucesso.\n", ctrl->nomeTarefa);

    while (true) {
        // 1. Verifica permissao de execucao ('m') ou encerramento ('ESC')
        if (!AguardarPermissaoThreadPrincipal(ctrl)) {
            break;
        }

        // 2. Gera dados aleatorios para Hora Prevista de Inicio, Duracao e Ordem de Producao (OP)
        int hh = GerarInteiroAleatorio(0, 23);
        int mm = GerarInteiroAleatorio(0, 59);
        int ss = GerarInteiroAleatorio(0, 59);
        std::snprintf(horaPrevista, sizeof(horaPrevista), "%02d:%02d:%02d", hh, mm, ss);

        int duracao = GerarInteiroAleatorio(10, 99999);

        // Formata a OP com 8 caracteres alfanumericos (ex: QPA-0098)
        char letra1 = static_cast<char>(GerarInteiroAleatorio('A', 'Z'));
        char letra2 = static_cast<char>(GerarInteiroAleatorio('A', 'Z'));
        char letra3 = static_cast<char>(GerarInteiroAleatorio('A', 'Z'));
        int numOP = GerarInteiroAleatorio(0, 9999);
        std::snprintf(op, sizeof(op), "%c%c%c-%04d", letra1, letra2, letra3, numOP);

        // 3. Monta a string exata do enunciado: 2/NSEQ/HORA/DURACAO/OP (31 caracteres)
        int len = std::snprintf(item.texto, sizeof(item.texto),
            "2/%05d/%s/%05d/%s",
            nseq, horaPrevista, duracao, op);

        if (len != TAM_MSG_PCP) {
            LogConsolePrincipal("[PCP] ALERTA: Mensagem com tamanho invalido (%d chars): %s\n",
                len, item.texto);
        }

        // 4. Deposita na Lista Circular 1
        if (!DepositarMensagemLista1(item, ctrl)) {
            break; // Termina imediatamente se o ESC for acionado durante bloqueio
        }

        // 5. Incrementa o sequencial (volta ao valor zero apos alcancar 99999, conforme enunciado)
        nseq++;
        if (nseq > NSEQ_MAX) {
            nseq = NSEQ_RESET;
        }

        // 6. Temporizacao provisoria da Etapa 1: Sleep de 1000 ms
        Sleep(TEMPO_ETAPA1_MS);
    }

    LogConsolePrincipal("[%s] Thread finalizada de forma limpa.\n", ctrl->nomeTarefa);
    return nullptr;
}