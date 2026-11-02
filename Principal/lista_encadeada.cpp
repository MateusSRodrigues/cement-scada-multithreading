#include "lista_encadeada.h"
#include <pthread.h>
#include <cstdio>
#include <cstdarg>

// Instancias globais da lista encadeada e dos mecanismos de sincronismo
static ListaEncadeada1 lista1;
static pthread_mutex_t mtxLista1;
static pthread_mutex_t mtxConsole;
static HANDLE hSemVagasL1 = nullptr;
static HANDLE hSemItensL1 = nullptr;

void InicializarLista1() {
    pthread_mutex_init(&mtxLista1, nullptr);
    pthread_mutex_init(&mtxConsole, nullptr);

    // Semaforo para contar vagas livres na lista (inicia com CAPACIDADE_LISTA_1 = 100)
    hSemVagasL1 = CreateSemaphoreA(nullptr, CAPACIDADE_LISTA_1, CAPACIDADE_LISTA_1, SEM_L1_VAGAS);

    // Semaforo para contar itens disponiveis na lista (inicia com 0)
    hSemItensL1 = CreateSemaphoreA(nullptr, 0, CAPACIDADE_LISTA_1, SEM_L1_ITENS);
}

void DestruirLista1() {
    // Esvazia a memoria de nos remanescentes (caso o encerramento ocorra com itens na lista)
    pthread_mutex_lock(&mtxLista1);
    Node* atual = lista1.head;
    while (atual != nullptr) {
        Node* prox = atual->next;
        delete atual;
        atual = prox;
    }
    lista1.head = nullptr;
    lista1.tail = nullptr;
    lista1.count = 0;
    pthread_mutex_unlock(&mtxLista1);

    pthread_mutex_destroy(&mtxLista1);
    pthread_mutex_destroy(&mtxConsole);

    if (hSemVagasL1) CloseHandle(hSemVagasL1);
    if (hSemItensL1) CloseHandle(hSemItensL1);
}

void LogConsolePrincipal(const char* formato, ...) {
    pthread_mutex_lock(&mtxConsole);
    va_list args;
    va_start(args, formato);
    std::vprintf(formato, args);
    va_end(args);
    std::fflush(stdout);
    pthread_mutex_unlock(&mtxConsole);
}

bool AguardarPermissaoThreadPrincipal(ControleTarefa* ctrl) {
    if (WaitForSingleObject(ctrl->hEvTerm, 0) == WAIT_OBJECT_0) {
        return false;
    }

    if (WaitForSingleObject(ctrl->hEvRun, 0) == WAIT_TIMEOUT) {
        LogConsolePrincipal("[%s] ESTADO: BLOQUEADA pelo comando do teclado. Aguardando liberacao ou ESC...\n",
            ctrl->nomeTarefa);

        HANDLE hEventos[2] = { ctrl->hEvTerm, ctrl->hEvRun };
        DWORD resultado = WaitForMultipleObjects(2, hEventos, FALSE, INFINITE);

        if (resultado == WAIT_OBJECT_0 || resultado == WAIT_FAILED) {
            LogConsolePrincipal("[%s] Notificacao de TERMINO (ESC) recebida durante bloqueio. Encerrando...\n",
                ctrl->nomeTarefa);
            return false;
        }

        if (WaitForSingleObject(ctrl->hEvTerm, 0) == WAIT_OBJECT_0) {
            return false;
        }

        LogConsolePrincipal("[%s] ESTADO: DESBLOQUEADA. Retomando execucao normal.\n",
            ctrl->nomeTarefa);
    }
    return true;
}

bool DepositarMensagemLista1(const ItemMensagem& item, ControleTarefa* ctrl) {
    HANDLE hEspera[2] = { ctrl->hEvTerm, hSemVagasL1 };
    bool avisoListaCheiaImpresso = false;

    while (true) {
        // 1. Garante que a tarefa nao esta bloqueada nem recebeu ESC
        if (!AguardarPermissaoThreadPrincipal(ctrl)) {
            return false;
        }

        // 2. Tenta obter 1 vaga imediatamente (timeout = 0)
        DWORD dwImmediate = WaitForSingleObject(hSemVagasL1, 0);
        if (dwImmediate == WAIT_OBJECT_0) {
            break; // Vaga adquirida imediatamente!
        }

        // Se nao havia vaga imediata, sinaliza Lista Cheia uma unica vez por bloqueio
        if (!avisoListaCheiaImpresso) {
            LogConsolePrincipal("[%s] Lista Circular 1 CHEIA (%d itens)! Aguardando vaga livre...\n",
                ctrl->nomeTarefa, CAPACIDADE_LISTA_1);
            avisoListaCheiaImpresso = true;
        }

        // 3. Aguarda vaga ou ESC com timeout curto (100 ms) para responder a bloqueios do teclado
        DWORD dwWait = WaitForMultipleObjects(2, hEspera, FALSE, 100);
        if (dwWait == WAIT_OBJECT_0 || dwWait == WAIT_FAILED) {
            return false; // ESC acionado
        }
        if (dwWait == WAIT_OBJECT_0 + 1) {
            // Se o usuario bloqueou a thread enquanto ela esperava vaga, devolve a vaga e bloqueia
            if (WaitForSingleObject(ctrl->hEvRun, 0) == WAIT_TIMEOUT) {
                ReleaseSemaphore(hSemVagasL1, 1, nullptr);
                continue;
            }
            break; // Vaga adquirida com tarefa desbloqueada!
        }
    }

    // Aloca o no fora da secao critica para minimizar o tempo de retencao do mutex
    Node* novoNo = new Node(item);

    // Exclusao mutua via Pthreads apenas para manipulacao dos ponteiros da lista
    pthread_mutex_lock(&mtxLista1);
    if (lista1.tail == nullptr) {
        lista1.head = novoNo;
        lista1.tail = novoNo;
    }
    else {
        lista1.tail->next = novoNo;
        lista1.tail = novoNo;
    }
    lista1.count++;
    pthread_mutex_unlock(&mtxLista1);

    // Sinaliza que ha um novo item disponivel para a Tarefa de Retirada
    ReleaseSemaphore(hSemItensL1, 1, nullptr);
    return true;
}

bool RetirarMensagemLista1(ItemMensagem* itemDestino, ControleTarefa* ctrl) {
    HANDLE hEspera[2] = { ctrl->hEvTerm, hSemItensL1 };

    while (true) {
        // 1. Garante que a Tarefa de Retirada nao foi bloqueada pela tecla 'r' nem recebeu ESC
        if (!AguardarPermissaoThreadPrincipal(ctrl)) {
            return false;
        }

        // 2. Aguarda item disponivel ou ESC com timeout de 100 ms.
        // O timeout curto permite detectar imediatamente a tecla 'r' mesmo com a lista vazia!
        DWORD dwWait = WaitForMultipleObjects(2, hEspera, FALSE, 100);
        if (dwWait == WAIT_OBJECT_0 || dwWait == WAIT_FAILED) {
            return false; // ESC acionado
        }

        if (dwWait == WAIT_OBJECT_0 + 1) {
            // Verifica se a tecla 'r' bloqueou a tarefa no exato instante em que o item chegou
            if (WaitForSingleObject(ctrl->hEvRun, 0) == WAIT_TIMEOUT) {
                ReleaseSemaphore(hSemItensL1, 1, nullptr);
                continue;
            }
            break; // Item consumido do semaforo com sucesso!
        }
    }

    // Exclusao mutua via Pthreads apenas para desengatar o no da cabeca da fila
    pthread_mutex_lock(&mtxLista1);
    Node* noRemovido = lista1.head;
    if (noRemovido != nullptr) {
        lista1.head = noRemovido->next;
        if (lista1.head == nullptr) {
            lista1.tail = nullptr;
        }
        lista1.count--;
    }
    pthread_mutex_unlock(&mtxLista1);

    if (noRemovido == nullptr) {
        return false;
    }

    // Copia os dados e libera a memoria fora da secao critica
    *itemDestino = noRemovido->item;
    delete noRemovido;

    // Sinaliza aos Produtores que uma nova vaga foi aberta na Lista 1
    ReleaseSemaphore(hSemVagasL1, 1, nullptr);
    return true;
}