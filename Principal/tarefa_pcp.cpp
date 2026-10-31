#include "tarefa_pcp.h"
#include "lista_circular1.h"

#include <random>
#include <cstdio>
#include <cstring>

// Gerador de números inteiros pseudoaleatórios
static int GerarInteiroAleatorio(int minVal, int maxVal) {
    // Semente local diferente da thread do supervisório para evitar padrões repetidos
    static thread_local std::mt19937 gerador(54321);
    std::uniform_int_distribution<int> dist(minVal, maxVal);
    return dist(gerador);
}

void* ThreadPCP(void* arg) {
    ControleTarefa* ctrl = static_cast<ControleTarefa*>(arg);
    if (!ctrl) {
        std::printf("[PCP] Erro: Ponteiro de controlo nulo!\n");
        return nullptr;
    }

    int nseq = NSEQ_MIN; // Inicia a contagem sequencial em 1[cite: 3, 9]
    char hora[16] = { 0 };
    char op[16] = { 0 };
    ItemMensagem item{};
    item.tipo = TIPO_ESCALONAMENTO_PCP; // Sempre tipo 2[cite: 3, 9]

    std::printf("[%s] Thread iniciada com sucesso.\n", ctrl->nomeTarefa);

    while (true) {
        // 1. Verifica permissão de execução ou encerramento pelo teclado[cite: 4, 9]
        if (!AguardarPermissaoExecucaoOuTermino(ctrl->hEvRun, ctrl->hEvTerm, ctrl->nomeTarefa)) {
            break;
        }

        // 2. Gera dados aleatórios para a duração (minutos) e Ordem de Produção[cite: 3]
        int duracao = GerarInteiroAleatorio(100, 9999);

        // Formata a OP com 8 caracteres (ex: QPA-0098)[cite: 3]
        char letra1 = (char)GerarInteiroAleatorio('A', 'Z');
        char letra2 = (char)GerarInteiroAleatorio('A', 'Z');
        char letra3 = (char)GerarInteiroAleatorio('A', 'Z');
        int numOP = GerarInteiroAleatorio(0, 9999);
        std::snprintf(op, sizeof(op), "%c%c%c-%04d", letra1, letra2, letra3, numOP);

        // 3. Obtém o timestamp corrente sem milissegundos (HH:MM:SS - 8 caracteres)[cite: 3, 9]
        ObterTimestampAtual(hora, sizeof(hora), false);

        // 4. Monta a string exata do enunciado: 2/NSEQ/HORA/DURACAO/OP[cite: 3]
        // Tamanho: 1 + 1 + 5 + 1 + 8 + 1 + 5 + 1 + 8 = 31 caracteres[cite: 3]
        int len = std::snprintf(item.texto, sizeof(item.texto),
            "2/%05d/%s/%05d/%s",
            nseq, hora, duracao, op);

        if (len != TAM_MSG_PCP) {
            std::printf("[PCP] ALERTA: Mensagem com tamanho invalido (%d chars): %s\n", len, item.texto);
        }

        // 5. Deposita na Lista Circular 1[cite: 3]
        if (!DepositarMensagemLista1(item, ctrl->hEvTerm, ctrl->nomeTarefa)) {
            break; // Termina imediatamente se o ESC for premido durante o bloqueio de lista cheia
        }

        // 6. Incrementa o sequencial (limite 99999)[cite: 3, 9]
        nseq++;
        if (nseq > NSEQ_MAX) {
            nseq = NSEQ_MIN;
        }

        // 7. Temporização provisória da Etapa 1: 1000 ms estáticos[cite: 4, 9]
        Sleep(TEMPO_ETAPA1_MS);
    }

    std::printf("[%s] Thread finalizada de forma limpa.\n", ctrl->nomeTarefa);
    return nullptr;
}
