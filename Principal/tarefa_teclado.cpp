#include "tarefa_teclado.h"
#include <conio.h> // Necessário para a função _getch()
#include <cstdio>

void ExecutarTarefaTeclado(HandlesTeclado* handles) {
    // Todas as tarefas iniciam no estado desbloqueado
    bool runSup = true, runPcp = true, runRet = true, runExib = true, runGest = true;

    std::printf("\n[TECLADO] Controle iniciado.\n");
    std::printf("-> Pressione 's', 'm', 'r', 'p' ou 'g' para ligar/desligar as tarefas.\n");
    std::printf("-> Pressione ESC para encerrar todas as threads.\n\n");

    while (true) {
        int tecla = _getch(); // Aguarda indefinidademente por um caractere do teclado

        // Encerramento global (ESC)
        if (tecla == TECLA_ENCERRAR_ESC) {
            std::printf("\n[TECLADO] Comando ESC detectado! Iniciando encerramento global...\n");
            
            // Dispara todos os eventos de término simultaneamente
            if (handles->hEvTermSup) SetEvent(handles->hEvTermSup);
            if (handles->hEvTermPCP) SetEvent(handles->hEvTermPCP);
            if (handles->hEvTermRet) SetEvent(handles->hEvTermRet);
            if (handles->hEvTermExib) SetEvent(handles->hEvTermExib);
            if (handles->hEvTermGest) SetEvent(handles->hEvTermGest);
            
            break; // Quebra o loop e devolve o controlo à main()
        }

        // Alternância de estado (Toggle) das tarefas
        switch (tecla) {
            case TECLA_BLK_SUPERVISORIO:
                if (!handles->hEvRunSup) break;
                runSup = !runSup;
                if (runSup) {
                    std::printf("[TECLADO] Supervisorio DESBLOQUEADO.\n");
                    SetEvent(handles->hEvRunSup);
                } else {
                    std::printf("[TECLADO] Supervisorio BLOQUEADO.\n");
                    ResetEvent(handles->hEvRunSup);
                }
                break;

            case TECLA_BLK_PCP:
                if (!handles->hEvRunPCP) break;
                runPcp = !runPcp;
                if (runPcp) {
                    std::printf("[TECLADO] PCP DESBLOQUEADO.\n");
                    SetEvent(handles->hEvRunPCP);
                } else {
                    std::printf("[TECLADO] PCP BLOQUEADO.\n");
                    ResetEvent(handles->hEvRunPCP);
                }
                break;

            case TECLA_BLK_RETIRADA:
                if (!handles->hEvRunRet) break;
                runRet = !runRet;
                if (runRet) SetEvent(handles->hEvRunRet); else ResetEvent(handles->hEvRunRet);
                std::printf("[TECLADO] Tarefa Retirada %s.\n", runRet ? "DESBLOQUEADA" : "BLOQUEADA");
                break;

            case TECLA_BLK_EXIB_PROC:
                if (!handles->hEvRunExib) break;
                runExib = !runExib;
                if (runExib) SetEvent(handles->hEvRunExib); else ResetEvent(handles->hEvRunExib);
                std::printf("[TECLADO] Exibicao de Processo %s.\n", runExib ? "DESBLOQUEADA" : "BLOQUEADA");
                break;

            case TECLA_BLK_GEST_PROD:
                if (!handles->hEvRunGest) break;
                runGest = !runGest;
                if (runGest) SetEvent(handles->hEvRunGest); else ResetEvent(handles->hEvRunGest);
                std::printf("[TECLADO] Gestao de Producao %s.\n", runGest ? "DESBLOQUEADA" : "BLOQUEADA");
                break;

            case TECLA_LIMPA_EXIB_PROC:
            case TECLA_LIMPA_GEST_PROD:
                std::printf("[TECLADO] Tecla '%c' registada (Comando IPC de limpeza sera enviado na Etapa 2).\n", tecla);
                break;

            default:
                break; // Ignora quaisquer outras teclas
        }
    }
}