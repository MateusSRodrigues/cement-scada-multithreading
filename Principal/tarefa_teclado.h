#ifndef TAREFA_TECLADO_H
#define TAREFA_TECLADO_H

#include "../include/common.h"

// Estrutura para agrupar os eventos de controlo criados na main()
struct HandlesTeclado {
    HANDLE hEvRunSup, hEvTermSup;
    HANDLE hEvRunPCP, hEvTermPCP;
    HANDLE hEvRunRet, hEvTermRet;
    HANDLE hEvRunExib, hEvTermExib;
    HANDLE hEvRunGest, hEvTermGest;
};

// Executa o loop de escuta do teclado (bloqueia a thread atual até pressionar ESC)
void ExecutarTarefaTeclado(HandlesTeclado* handles);

#endif // TAREFA_TECLADO_H