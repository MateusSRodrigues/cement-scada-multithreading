#include "tarefa_supervisorio.h"
#include "lista_encadeada.h"
#include <random>
#include <cstdio>
#include <cstring>

// Gerador de numeros pseudoaleatorios em ponto flutuante com semente dinamica
static float GerarRealAleatorio(float minVal, float maxVal) {
    static thread_local std::mt19937 gerador(std::random_device{}() ^ GetCurrentThreadId());
    std::uniform_real_distribution<float> dist(minVal, maxVal);
    return dist(gerador);
}

void* ThreadSupervisorio(void* arg) {
    ControleTarefa* ctrl = static_cast<ControleTarefa*>(arg);
    if (!ctrl) {
        LogConsolePrincipal("[SUPERVISORIO] Erro: Ponteiro de controle nulo!\n");
        return nullptr;
    }

    int nseq = NSEQ_MIN; // Inicia em 1
    char timestamp[16] = { 0 };
    ItemMensagem item{};
    item.tipo = TIPO_DADOS_PROCESSO; // Sempre tipo 1

    LogConsolePrincipal("[%s] Thread iniciada com sucesso.\n", ctrl->nomeTarefa);

    while (true) {
        // 1. Verifica se a tarefa foi bloqueada pela tecla 's' ou se recebeu ESC
        if (!AguardarPermissaoThreadPrincipal(ctrl)) {
            break;
        }

        // 2. Gera valores aleatorios nas faixas com formato NNN.N (5 caracteres cada)
        float tz1 = GerarRealAleatorio(100.0f, 999.9f); // Temperatura Zona 1 (C)
        float tz2 = GerarRealAleatorio(100.0f, 999.9f); // Temperatura Zona 2 (C)
        float tz3 = GerarRealAleatorio(100.0f, 999.9f); // Temperatura Zona 3 (C)
        float vol = GerarRealAleatorio(100.0f, 999.9f); // Volume de clinquer (m3)
        float pres = GerarRealAleatorio(10.0f, 99.9f);  // Pressao interna (kgf/m2) -> 0NN.N

        // 3. Obtem o timestamp corrente com precisao de milissegundos (HH:MM:SS.NNN - 12 chars)
        ObterTimestampAtual(timestamp, sizeof(timestamp), true);

        // 4. Monta a string no formato exato: TIPO/NSEQ/TZ1/TZ2/TZ3/VOL/PRES/TIMESTAMP (50 chars)
        int len = std::snprintf(item.texto, sizeof(item.texto),
            "1/%05d/%05.1f/%05.1f/%05.1f/%05.1f/%05.1f/%s",
            nseq, tz1, tz2, tz3, vol, pres, timestamp);

        if (len != TAM_MSG_SUPERVISORIO) {
            LogConsolePrincipal("[SUPERVISORIO] ALERTA: Mensagem com tamanho invalido (%d chars): %s\n",
                len, item.texto);
        }

        // 5. Deposita a mensagem na Lista Circular 1 (bloqueia com aviso se estiver cheia)
        if (!DepositarMensagemLista1(item, ctrl)) {
            break; // Recebeu ESC enquanto aguardava
        }

        // 6. Incrementa o sequencial (volta ao valor zero apos alcancar 99999, conforme enunciado)
        nseq++;
        if (nseq > NSEQ_MAX) {
            nseq = NSEQ_RESET;
        }

        // 7. Temporizacao provisoria da Etapa 1: Sleep de 1000 ms
        Sleep(TEMPO_ETAPA1_MS);
    }

    LogConsolePrincipal("[%s] Thread finalizada de forma limpa.\n", ctrl->nomeTarefa);
    return nullptr;
}