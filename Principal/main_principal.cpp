#include "common.h"
#include "tarefa_pcp.h" 
#include "tarefa_supervisorio.h"
#include "tarefa_teclado.h"

#include <pthread.h>
#include <cstdio>

int main(void) {

    std::printf("Iniciando o Processo Principal...\n");

    // hEvRunSup inicia como TRUE (Sinalizado = rodando)
    HANDLE hEvRunSup = CreateEventA(NULL, TRUE, TRUE, EV_RUN_SUPERVISORIO);
    // hEvTermSup inicia como FALSE (Não-Sinalizado = não deve encerrar ainda)
    HANDLE hEvTermSup = CreateEventA(NULL, TRUE, FALSE, EV_TERM_SUPERVISORIO);

    // Criar eventos e controlo para o PCP
    HANDLE hEvRunPCP = CreateEventA(NULL, TRUE, TRUE, EV_RUN_PCP);
    HANDLE hEvTermPCP = CreateEventA(NULL, TRUE, FALSE, EV_TERM_PCP);

    ControleTarefa ctrlSupervisorio;
    ctrlSupervisorio.hEvRun = hEvRunSup;        // Evento Manual-Reset criado com TRUE
    ctrlSupervisorio.hEvTerm = hEvTermSup;      // Evento Manual-Reset criado com FALSE
    ctrlSupervisorio.nomeTarefa = "SUPERVISORIO";

    pthread_t tSupervisorio;
    int status = pthread_create(&tSupervisorio, nullptr, ThreadSupervisorio, &ctrlSupervisorio);
    if (status != 0) {
        std::printf("Falha ao criar thread do Supervisorio!\n");
    }  

    ControleTarefa ctrlPCP;
    ctrlPCP.hEvRun = hEvRunPCP;
    ctrlPCP.hEvTerm = hEvTermPCP;
    ctrlPCP.nomeTarefa = "SISTEMA_PCP";

    pthread_t tPCP;
    if (pthread_create(&tPCP, nullptr, ThreadPCP, &ctrlPCP) != 0) {
        std::printf("Erro fatal: Falha ao criar thread do PCP!\n");
    }

    // Cria a estrutura que centraliza os handles para o teclado
    HandlesTeclado handles = {};
    handles.hEvRunSup = hEvRunSup;
    handles.hEvTermSup = hEvTermSup;
    handles.hEvRunPCP = hEvRunPCP;
    handles.hEvTermPCP = hEvTermPCP;

    // A função bloqueará a thread principal lendo o teclado
    // Só retornará quando o utilizador pressionar ESC
    ExecutarTarefaTeclado(&handles);

    // Quando o ESC for pressionado, os eventos de término já foram sinalizados
    std::printf("Aguardando encerramento das threads...\n");
    pthread_join(tSupervisorio, nullptr);
    pthread_join(tPCP, nullptr);

    CloseHandle(hEvRunSup);
    CloseHandle(hEvTermSup);
    CloseHandle(hEvRunPCP);
    CloseHandle(hEvTermPCP);

    //pthread_join(tSupervisorio, nullptr);
    //SetEvent(hEvTermPCP);
    //pthread_join(tPCP, nullptr);
    //CloseHandle(hEvRunPCP);
    //CloseHandle(hEvTermPCP);

    std::printf("Processo Principal encerrado com sucesso.\n");
    return 0;
    
}