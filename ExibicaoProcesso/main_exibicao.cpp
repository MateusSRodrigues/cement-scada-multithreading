#include "../include/common.h"
#include <cstdio>

int main() {
    SetConsoleTitleA("Tela de Exibicao de Dados de Processo (Etapa 1)");

    std::printf("==============================================================\n");
    std::printf("        TELA DE EXIBICAO DE DADOS DE PROCESSO (ETAPA 1)       \n");
    std::printf("==============================================================\n\n");

    // 1. Abre os Eventos Nomeados criados pelo Processo Principal
    HANDLE hEvRun = OpenEventA(SYNCHRONIZE, FALSE, EV_RUN_EXIB_PROC);
    HANDLE hEvTerm = OpenEventA(SYNCHRONIZE, FALSE, EV_TERM_EXIB_PROC);

    if (!hEvRun || !hEvTerm) {
        std::printf("[EXIBICAO_PROC] ERRO: Nao foi possivel abrir os eventos de sincronizacao (Erro Win32: %lu).\n",
            GetLastError());
        std::printf("[EXIBICAO_PROC] Certifique-se de que este executavel foi disparado pelo Principal.exe.\n");
        Sleep(4000);
        return 1;
    }

    std::printf("[EXIBICAO_PROC] ESTADO: DESBLOQUEADA (Em execucao normal).\n");
    std::printf("[EXIBICAO_PROC] Aguardando comandos ('p' para bloquear/desbloquear, 'ESC' para encerrar)...\n\n");

    // 2. Loop principal de sincronizacao da Etapa 1 (sem busy-waiting e sem poluicao visual)
    while (true) {
        // Se hEvRun for desligado pela tecla 'p', imprime ESTADO: BLOQUEADA e aguarda liberacao ou ESC
        if (!AguardarPermissaoExecucaoOuTermino(hEvRun, hEvTerm, "EXIBICAO_PROC")) {
            break;
        }

        // Aguarda passivamente pelo evento de termino (ESC) com timeout de 200 ms
        // para verificar rapidamente se a tecla 'p' alterou o estado de hEvRun
        DWORD res = WaitForSingleObject(hEvTerm, 200);
        if (res == WAIT_OBJECT_0) {
            std::printf("[EXIBICAO_PROC] Notificacao de TERMINO (ESC) recebida. Encerrando...\n");
            break;
        }
    }

    // 3. Libera os handles locais antes de encerrar o processo
    CloseHandle(hEvRun);
    CloseHandle(hEvTerm);

    std::printf("[EXIBICAO_PROC] Processo finalizado com sucesso.\n");
    return 0;
}