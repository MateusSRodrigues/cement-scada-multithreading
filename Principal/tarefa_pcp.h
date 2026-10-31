#ifndef TAREFA_PCP_H
#define TAREFA_PCP_H

#include "../include/common.h"
#include <pthread.h>

// Função de entrada da thread do PCP (compatível com pthread_create)
void* ThreadPCP(void* arg);

#endif // TAREFA_PCP_H
