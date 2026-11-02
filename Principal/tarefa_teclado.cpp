#include "tarefa_teclado.h"
#include "lista_encadeada.h"
#include <conio.h>
#include <cstdio>
#include <cctype>

void ExecutarTarefaTeclado(HandlesTeclado* handles) {
    // Todas as tarefas iniciam com os bloqueios desligados (plena execucao)
    bool runSup = true, runPcp = true, runRet = true, runExib = true, runGest = true;

    LogConsolePrincipal("\n[TECLADO] Controle iniciado (todas as tarefas desbloqueadas).\n");
    LogConsolePrincipal("-> Pressione 's' (Supervisorio), 'm' (PCP), 'r' (Retirada), 'p' (Exibicao) ou 'g' (Gestao) para bloquear/desbloquear.\n");
    LogConsolePrincipal("-> Pressione '1' ou '2' para limpeza de tela (Etapa 2).\n");
    LogConsolePrincipal("-> Pressione ESC para encerrar toda a aplicacao.\n\n");

    while (true) {
        int tecla = _getch();

        // Ignora o prefixo de teclas especiais/setas do teclado (0 ou 0xE0)
        if (tecla == 0 || tecla == 0xE0) {
            (void)_getch();
            continue;
        }

        // Encerramento global (ESC)
        if (tecla == TECLA_ENCERRAR_ESC) {
            LogConsolePrincipal("\n[TECLADO] Comando ESC detectado! Sinalizando encerramento global...\n");

            if (handles->hEvTermSup)  SetEvent(handles->hEvTermSup);
            if (handles->hEvTermPCP)  SetEvent(handles->hEvTermPCP);
            if (handles->hEvTermRet)  SetEvent(handles->hEvTermRet);
            if (handles->hEvTermExib) SetEvent(handles->hEvTermExib);
            if (handles->hEvTermGest) SetEvent(handles->hEvTermGest);

            break;
        }

        // Converte para minuscula para suportar Caps Lock ligado
        char cmd = static_cast<char>(std::tolower(static_cast<unsigned char>(tecla)));

        switch (cmd) {
        case TECLA_BLK_SUPERVISORIO: // 's'
            if (!handles->hEvRunSup) break;
            runSup = !runSup;
            if (runSup) SetEvent(handles->hEvRunSup); else ResetEvent(handles->hEvRunSup);
            LogConsolePrincipal("[TECLADO] Comando 's': Tarefa do Supervisorio %s.\n",
                runSup ? "DESBLOQUEADA" : "BLOQUEADA");
            break;

        case TECLA_BLK_PCP: // 'm'
            if (!handles->hEvRunPCP) break;
            runPcp = !runPcp;
            if (runPcp) SetEvent(handles->hEvRunPCP); else ResetEvent(handles->hEvRunPCP);
            LogConsolePrincipal("[TECLADO] Comando 'm': Tarefa do PCP %s.\n",
                runPcp ? "DESBLOQUEADA" : "BLOQUEADA");
            break;

        case TECLA_BLK_RETIRADA: // 'r'
            if (!handles->hEvRunRet) break;
            runRet = !runRet;
            if (runRet) SetEvent(handles->hEvRunRet); else ResetEvent(handles->hEvRunRet);
            LogConsolePrincipal("[TECLADO] Comando 'r': Tarefa de Retirada %s.\n",
                runRet ? "DESBLOQUEADA" : "BLOQUEADA");
            break;

        case TECLA_BLK_EXIB_PROC: // 'p'
            if (!handles->hEvRunExib) break;
            runExib = !runExib;
            if (runExib) SetEvent(handles->hEvRunExib); else ResetEvent(handles->hEvRunExib);
            LogConsolePrincipal("[TECLADO] Comando 'p': Exibicao de Dados de Processo %s.\n",
                runExib ? "DESBLOQUEADA" : "BLOQUEADA");
            break;

        case TECLA_BLK_GEST_PROD: // 'g'
            if (!handles->hEvRunGest) break;
            runGest = !runGest;
            if (runGest) SetEvent(handles->hEvRunGest); else ResetEvent(handles->hEvRunGest);
            LogConsolePrincipal("[TECLADO] Comando 'g': Gestao da Producao %s.\n",
                runGest ? "DESBLOQUEADA" : "BLOQUEADA");
            break;

        case TECLA_LIMPA_EXIB_PROC: // '1'
        case TECLA_LIMPA_GEST_PROD: // '2'
            LogConsolePrincipal("[TECLADO] Tecla '%c' registrada (IPC de limpeza de console sera ativado na Etapa 2).\n", cmd);
            break;

        default:
            break;
        }
    }
}