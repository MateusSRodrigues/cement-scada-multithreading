#ifndef TAREFA_SUPERVISORIO_H
#define TAREFA_SUPERVISORIO_H

#include "../include/common.h"
#include <pthread.h>

// Função de entrada da thread do Supervisório (compatível com pthread_create)
void* ThreadSupervisorio(void* arg);

#endif // TAREFA_SUPERVISORIO_H