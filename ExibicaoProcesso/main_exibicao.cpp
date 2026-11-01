#include "../include/common.h"

int main() {
    // Define o título da janela para facilitar a identificação
    SetConsoleTitleA("Tela de Dados de Processo (Etapa 1)");

    printf("Iniciando Tela de Dados de Processo...\n");

    // Abre os eventos criados previamente pelo processo Principal
    HANDLE hEvRun = OpenEventA(SYNCHRONIZE, FALSE, EV_RUN_EXIB_PROC);
    HANDLE hEvTerm = OpenEventA(SYNCHRONIZE, FALSE, EV_TERM_EXIB_PROC);

    if (hEvRun == nullptr || hEvTerm == nullptr) {
        printf("ERRO: Eventos nao encontrados. Execute o Principal.exe primeiro!\n");
        system("pause");
        return 1;
    }

    // Ciclo de vida da janela de exibição
    while (true) {
        // Verifica se a tecla 'p' bloqueou a tela ou se 'ESC' foi premido
        if (!AguardarPermissaoExecucaoOuTermino(hEvRun, hEvTerm, "EXIBICAO PROCESSO")) {
            break; // Sai do ciclo se receber o comando de encerramento
        }

        // Na Etapa 1, apenas simulamos a atividade da tela
        printf("[EXIBICAO] Tela ativa e aguardando dados da Etapa 2...\n");

        Sleep(TEMPO_ETAPA1_MS);
    }

    printf("Encerrando Tela de Dados de Processo...\n");

    // Liberta os handles locais
    CloseHandle(hEvRun);
    CloseHandle(hEvTerm);

    return 0;
}