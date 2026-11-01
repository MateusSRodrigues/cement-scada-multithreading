#include "../include/common.h"
#include <cstdio>

int main() {
    SetConsoleTitleA("Tela de Acompanhamento de Producao (Gestao PCP)");

    std::printf("==============================================================\n");
    std::printf("       TELA DE ACOMPANHAMENTO DE PRODUCAO (ETAPA 1)           \n");
    std::printf("==============================================================\n\n");

    // 1. Abre os Eventos Nomeados ja criados pelo Processo Principal
    HANDLE hEvRun = OpenEventA(SYNCHRONIZE, FALSE, EV_RUN_GEST_PROD);
    HANDLE hEvTerm = OpenEventA(SYNCHRONIZE, FALSE, EV_TERM_GEST_PROD);

    if (!hEvRun || !hEvTerm) {
        std::printf("[GESTAO_PROD] ERRO: Nao foi possivel abrir os eventos de sincronizacao (Erro Win32: %lu).\n",
            GetLastError());
        std::printf("[GESTAO_PROD] Certifique-se de que este processo foi disparado pelo Principal.exe.\n");
        Sleep(4000);
        return 1;
    }

    std::printf("[GESTAO_PROD] ESTADO: DESBLOQUEADA (Em execucao normal).\n");
    std::printf("[GESTAO_PROD] Aguardando comandos ('g' para bloquear/desbloquear, 'ESC' para encerrar)...\n\n");

    // 2. Loop principal de sincronizacao da Etapa 1
    while (true) {
        // Se hEvRun for desligado (ResetEvent pela tecla 'g'), a funcao imprime o estado BLOQUEADA
        // e aguarda ate ser desbloqueada novamente ('g') ou ate receber ordem de termino ('ESC').
        if (!AguardarPermissaoExecucaoOuTermino(hEvRun, hEvTerm, "GESTAO_PROD")) {
            break; // Recebeu ESC enquanto estava bloqueada ou no inicio do ciclo
        }

        // Enquanto estiver desbloqueada, aguarda passivamente pelo evento de termino (ESC)
        // com um pequeno timeout (200 ms) para revalidar se o estado de bloqueio mudou
        DWORD res = WaitForSingleObject(hEvTerm, 200);
        if (res == WAIT_OBJECT_0) {
            std::printf("[GESTAO_PROD] Notificacao de TERMINO (ESC) recebida. Encerrando...\n");
            break;
        }
    }

    // 3. Liberta os handles locais antes de fechar a janela
    CloseHandle(hEvRun);
    CloseHandle(hEvTerm);

    std::printf("[GESTAO_PROD] Processo finalizado com sucesso.\n");
    return 0;
}