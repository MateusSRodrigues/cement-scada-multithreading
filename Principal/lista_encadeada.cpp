#include "common.h"
#include <pthread.h>

// Instâncias globais da lista encadeada e dos mecanismos de sincronismo
ListaEncadeada1 lista1;
pthread_mutex_t mtxLista1;
HANDLE hSemVagasL1;
HANDLE hSemItensL1;

void InicializarLista1() {
    pthread_mutex_init(&mtxLista1, nullptr);

    // Semáforo para contar vagas livres na lista (inicia com CAPACIDADE_LISTA_1)
    hSemVagasL1 = CreateSemaphoreA(nullptr, CAPACIDADE_LISTA_1, CAPACIDADE_LISTA_1, SEM_L1_VAGAS);

    // Semáforo para contar itens disponíveis na lista (inicia com 0)
    hSemItensL1 = CreateSemaphoreA(nullptr, 0, CAPACIDADE_LISTA_1, SEM_L1_ITENS);
}

void DestruirLista1() {
    // Esvazia a memória de nós remanescentes (caso o encerramento ocorra com lista cheia)
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
    CloseHandle(hSemVagasL1);
    CloseHandle(hSemItensL1);
}

bool DepositarMensagemLista1(ItemMensagem item, HANDLE hEvTerm, const char* nomeProdutor) {

    HANDLE hEspera[2] = { hEvTerm, hSemVagasL1 };

    // Se não há vaga (timeout = 0), apenas avisa no console, depois aguarda bloqueado
    if (WaitForSingleObject(hSemVagasL1, 0) == WAIT_TIMEOUT) {
        printf("[%s] Lista Cheia! Aguardando vaga para depositar...\n", nomeProdutor);
        fflush(stdout);
    }

    // Aguarda vagar espaço (hSemVagasL1) OU o comando de encerramento (hEvTerm)
    DWORD dwWait = WaitForMultipleObjects(2, hEspera, FALSE, INFINITE);
    if (dwWait == WAIT_OBJECT_0) {
        // Encerramento (ESC) acionado enquanto esperava vaga
        return false;
    }

    // Exclusão Mútua para manipulação dos ponteiros
    pthread_mutex_lock(&mtxLista1);

    Node* novoNo = new Node(item);
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

    // Sinaliza que há um novo item disponível para o Consumidor
    ReleaseSemaphore(hSemItensL1, 1, nullptr);

    return true;
}

bool RetirarMensagemLista1(ItemMensagem* itemDestino, HANDLE hEvTerm) {
    HANDLE hEspera[2] = { hEvTerm, hSemItensL1 };

    // Aguarda um item ser depositado (hSemItensL1) OU o comando de encerramento (hEvTerm)
    DWORD dwWait = WaitForMultipleObjects(2, hEspera, FALSE, INFINITE);
    if (dwWait == WAIT_OBJECT_0) {
        return false;
    }

    pthread_mutex_lock(&mtxLista1);

    Node* noRemovido = lista1.head;
    *itemDestino = noRemovido->item; // Copia a string e o tipo da mensagem

    lista1.head = noRemovido->next;
    if (lista1.head == nullptr) {
        lista1.tail = nullptr;
    }
    lista1.count--;

    delete noRemovido; // Libera a memória alocada via new

    pthread_mutex_unlock(&mtxLista1);

    // Sinaliza aos Produtores que uma nova vaga foi aberta
    ReleaseSemaphore(hSemVagasL1, 1, nullptr);

    return true;
}