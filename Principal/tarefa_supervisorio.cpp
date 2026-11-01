#include "tarefa_supervisorio.h"
#include "lista_circular1.h"

#include <random>
#include <cstdio>
#include <cstring>

// Gerador de números pseudoaleatórios em ponto flutuante
static float GerarRealAleatorio(float minVal, float maxVal) {
    static thread_local std::mt19937 gerador(12345); // Semente local da thread
    std::uniform_real_distribution<float> dist(minVal, maxVal);
    return dist(gerador);
}

void* ThreadSupervisorio(void* arg) {
    // 1. Recupera os handles de sincronização passados pelo Processo Principal
    ControleTarefa* ctrl = static_cast<ControleTarefa*>(arg);
    if (!ctrl) {
        std::printf("[SUPERVISORIO] Erro: Ponteiro de controle nulo!\n");
        return nullptr;
    }

    int nseq = NSEQ_MIN; // Inicia em 1
    char timestamp[16] = { 0 };
    ItemMensagem item{};
    item.tipo = TIPO_DADOS_PROCESSO; // Sempre tipo 1[cite: 2]

    std::printf("[%s] Thread iniciada com sucesso.\n", ctrl->nomeTarefa);

    while (true) {
        // 2. Verifica se a tarefa foi bloqueada pela tecla 's' ou se recebeu ESC
        if (!AguardarPermissaoExecucaoOuTermino(ctrl->hEvRun, ctrl->hEvTerm, ctrl->nomeTarefa)) {
            break; // Comando ESC detectado: encerra a thread imediatamente[cite: 2, 9]
        }

        // 3. Gera valores aleatórios coerentes com o processo industrial de cimento[cite: 2]
        // Faixas simuladas com formato fixo NNN.N (5 caracteres cada)[cite: 2]:
        float tz1  = GerarRealAleatorio(100.0f, 999.9f); // Zona 1 (°C)[cite: 2]
        float tz2  = GerarRealAleatorio(100.0f, 999.9f); // Zona 2 (°C)[cite: 2]
        float tz3  = GerarRealAleatorio(100.0f, 999.9f); // Zona 3 (°C)[cite: 2]
        float vol  = GerarRealAleatorio(100.0f, 999.9f); // Volume (m³)[cite: 2]
        float pres = GerarRealAleatorio(10.0f,  99.9f);  // Pressão (kgf/m²) formatada como 0NN.N[cite: 2]

        // 4. Obtém o timestamp corrente com precisão de milissegundos (HH:MM:SS.MSS - 12 chars)[cite: 2, 9]
        ObterTimestampAtual(timestamp, sizeof(timestamp), true);

        // 5. Monta a string no formato exato: TIPO/NSEQ/TZ1/TZ2/TZ3/VOL/PRES/TIMESTAMP[cite: 2]
        // Tamanhos: 1 + 1 + 5 + 1 + 5 + 1 + 5 + 1 + 5 + 1 + 5 + 1 + 5 + 1 + 12 = 50 caracteres[cite: 2]
        int len = std::snprintf(item.texto, sizeof(item.texto),
                                "1/%05d/%05.1f/%05.1f/%05.1f/%05.1f/%05.1f/%s",
                                nseq, tz1, tz2, tz3, vol, pres, timestamp);

        // Validação defensiva do tamanho exigido pelo enunciado[cite: 2]
        if (len != TAM_MSG_SUPERVISORIO) {
            std::printf("[SUPERVISORIO] ALERTA: Mensagem com tamanho invalido (%d chars): %s\n", len, item.texto);
        }

        // 6. Deposita a mensagem na Lista Circular 1 (bloqueia com aviso se estiver cheia)[cite: 2]
        if (!DepositarMensagemLista1(item, ctrl->hEvTerm, ctrl->nomeTarefa)) {
            // Se retornou false, o usuário pressionou ESC enquanto a lista estava cheia
            break;
        }

        // 7. Incrementa o sequencial (volta a 1 após 99999)[cite: 2]
        nseq++;
        if (nseq > NSEQ_MAX) {
            nseq = NSEQ_MIN;
        }

        // 8. Temporização provisória da Etapa 1: Sleep de 1000 ms[cite: 4, 9]
        // (Na Etapa 2, será substituído por Waitable Timer de 500 ms sem Sleep)[cite: 2, 5]
        Sleep(TEMPO_ETAPA1_MS);
    }

    std::printf("[%s] Thread finalizada de forma limpa.\n", ctrl->nomeTarefa);
    return nullptr;
}