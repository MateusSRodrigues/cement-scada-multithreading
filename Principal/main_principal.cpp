#include "../include/common.h"
#include "lista_circular1.h"
#include "tarefa_supervisorio.h"
#include "tarefa_pcp.h"
#include "tarefa_retirada.h"
#include "tarefa_teclado.h"

#include <pthread.h>
#include <cstdio>

// Função para disparar  processos filhos
static bool DispararProcessoFilho(const char* nomeExecutavel, PROCESS_INFORMATION* piOut) {
    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(piOut, sizeof(*piOut));

    char cmdLine[MAX_PATH];
    std::snprintf(cmdLine, sizeof(cmdLine), "%s", nomeExecutavel);

    BOOL sucesso = CreateProcessA(
        nullptr,            // Nome do módulo (usa cmdLine)
        cmdLine,            // Linha de comando
        nullptr,            // Atributos de segurança do processo
        nullptr,            // Atributos de segurança da thread
        FALSE,              // Herança de handles (usamos eventos nomeados, não precisa herdar)
        CREATE_NEW_CONSOLE, // Cria uma janela de console separada!
        nullptr,            // Ambiente padrão
        nullptr,            // Mesmo diretório corrente (bin\Debug)
        &si,
        piOut
    );

    if (!sucesso) {
        std::printf("[PRINCIPAL] AVISO: Nao foi possivel iniciar '%s' (Erro Win32: %lu).\n",
            nomeExecutavel, GetLastError());
        return false;
    }

    std::printf("[PRINCIPAL] Processo '%s' disparado com sucesso (PID: %lu).\n",
        nomeExecutavel, piOut->dwProcessId);
    return true;
}

int main(){
    SetConsoleTitleA("Console Principal - Controle do Teclado e Log");
    std::printf("==============================================================\n");
    std::printf("  SISTEMA SUPERVISORIO E PCP - INDUSTRIA DE CIMENTO \n");
    std::printf("==============================================================\n\n");

    // Cria os 5 pares de Eventos Nomeados do Kernel Win32
    // Eventos de Execução (Run): Manual-Reset = TRUE, Estado Inicial = TRUE (Desbloqueados)
    // Eventos de Término (Term): Manual-Reset = TRUE, Estado Inicial = FALSE
    HandlesTeclado h = {};
    h.hEvRunSup = CreateEventA(nullptr, TRUE, TRUE, EV_RUN_SUPERVISORIO);
    h.hEvTermSup = CreateEventA(nullptr, TRUE, FALSE, EV_TERM_SUPERVISORIO);

    h.hEvRunPCP = CreateEventA(nullptr, TRUE, TRUE, EV_RUN_PCP);
    h.hEvTermPCP = CreateEventA(nullptr, TRUE, FALSE, EV_TERM_PCP);

    h.hEvRunRet = CreateEventA(nullptr, TRUE, TRUE, EV_RUN_RETIRADA);
    h.hEvTermRet = CreateEventA(nullptr, TRUE, FALSE, EV_TERM_RETIRADA);

    h.hEvRunExib = CreateEventA(nullptr, TRUE, TRUE, EV_RUN_EXIB_PROC);
    h.hEvTermExib = CreateEventA(nullptr, TRUE, FALSE, EV_TERM_EXIB_PROC);

    h.hEvRunGest = CreateEventA(nullptr, TRUE, TRUE, EV_RUN_GEST_PROD);
    h.hEvTermGest = CreateEventA(nullptr, TRUE, FALSE, EV_TERM_GEST_PROD);

    if (!h.hEvRunSup || !h.hEvTermSup || !h.hEvRunPCP || !h.hEvTermPCP ||
        !h.hEvRunRet || !h.hEvTermRet || !h.hEvRunExib || !h.hEvTermExib ||
        !h.hEvRunGest || !h.hEvTermGest) {
        std::printf("[PRINCIPAL] ERRO FATAL: Falha ao criar eventos do Kernel Win32.\n");
        return 1;
    }

    //Inicializa a Lista Circular 1 em memória RAM (100 posições)
    InicializarLista1();

    // Dispara os 2 processos filhos (Telas de Exibição de Processo e Gestão da Produção)
    PROCESS_INFORMATION piExib = {}, piGest = {};
    bool exibRodando = DispararProcessoFilho("ExibicaoProcesso.exe", &piExib);
    bool gestRodando = DispararProcessoFilho("GestaoProducao.exe", &piGest);

    // Prepara estruturas de controle para as 3 threads filhas do Processo Principal
    ControleTarefa ctrlSup = { h.hEvRunSup, h.hEvTermSup, "SUPERVISORIO" };
    ControleTarefa ctrlPCP = { h.hEvRunPCP, h.hEvTermPCP, "SISTEMA_PCP" };
    ControleTarefa ctrlRet = { h.hEvRunRet, h.hEvTermRet, "RETIRADA_MSG" };

    //  Cria as 3 threads usando Pthreads-Win32
    pthread_t tSup, tPCP, tRet;
    if (pthread_create(&tSup, nullptr, ThreadSupervisorio, &ctrlSup) != 0 ||
        pthread_create(&tPCP, nullptr, ThreadPCP, &ctrlPCP) != 0  ||
        pthread_create(&tRet, nullptr, ThreadRetirada, &ctrlRet) != 0) {
        std::printf("[PRINCIPAL] ERRO FATAL: Falha ao criar threads via Pthreads-Win32!\n");
        return 1;
    }

    // Executa a Tarefa de Leitura do Teclado na thread principal
    // (Fica bloqueada aqui até o usuário pressionar ESC)
    ExecutarTarefaTeclado(&h);

    //  Sincroniza o encerramento das threads locais após ESC
    std::printf("[PRINCIPAL] Aguardando encerramento das threads locais...\n");
    pthread_join(tSup, nullptr);
    pthread_join(tPCP, nullptr);
    //pthread_join(tRet, nullptr);

    // Aguarda o encerramento dos processos filhos das duas telas
    if (exibRodando) {
        std::printf("[PRINCIPAL] Aguardando fechamento da janela ExibicaoProcesso...\n");
        WaitForSingleObject(piExib.hProcess, 5000);
        CloseHandle(piExib.hThread);
        CloseHandle(piExib.hProcess);
    }
    if (gestRodando) {
        std::printf("[PRINCIPAL] Aguardando fechamento da janela GestaoProducao...\n");
        WaitForSingleObject(piGest.hProcess, 5000);
        CloseHandle(piGest.hThread);
        CloseHandle(piGest.hProcess);
    }

    //Libera a Lista Circular 1 e fecha todos os Handles do Kernel
    DestruirLista1();

    CloseHandle(h.hEvRunSup);  CloseHandle(h.hEvTermSup);
    CloseHandle(h.hEvRunPCP);  CloseHandle(h.hEvTermPCP);
    CloseHandle(h.hEvRunRet);  CloseHandle(h.hEvTermRet);
    CloseHandle(h.hEvRunExib); CloseHandle(h.hEvTermExib);
    CloseHandle(h.hEvRunGest); CloseHandle(h.hEvTermGest);

    std::printf("[PRINCIPAL] Todos os recursos liberados. Aplicacao encerrada com sucesso!\n");
    return 0;
}