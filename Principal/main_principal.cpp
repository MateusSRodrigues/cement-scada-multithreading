#include "common.h"
#include "tarefa_pcp.h" 

#include <pthread.h>
#include <cstdio>

int main(void) {

    std::printf("Iniciando o Processo Principal...\n");

    // Criar eventos e controlo para o PCP
    HANDLE hEvRunPCP = CreateEventA(NULL, TRUE, TRUE, EV_RUN_PCP);
    HANDLE hEvTermPCP = CreateEventA(NULL, TRUE, FALSE, EV_TERM_PCP);

    ControleTarefa ctrlPCP;
    ctrlPCP.hEvRun = hEvRunPCP;
    ctrlPCP.hEvTerm = hEvTermPCP;
    ctrlPCP.nomeTarefa = "SISTEMA_PCP";

    pthread_t tPCP;
    if (pthread_create(&tPCP, nullptr, ThreadPCP, &ctrlPCP) != 0) {
        std::printf("Erro fatal: Falha ao criar thread do PCP!\n");
    }

    //SetEvent(hEvTermPCP);
    pthread_join(tPCP, nullptr);
    //CloseHandle(hEvRunPCP);
    //CloseHandle(hEvTermPCP);

    std::printf("Processo Principal encerrado com sucesso.\n");
    return 0;
    
}