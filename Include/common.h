// Arquivo: common.h
// Descricao: Cabecalho compartilhado entre os 3 processos da aplicacao.
// Disciplina: Automacao em Tempo Real (ELT127) - DELT/UFMG

#ifndef COMMON_H
#define COMMON_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

// Constantes de tamanho e limites
#define TAM_MSG_SUPERVISORIO    50
#define TAM_MSG_PCP             31
#define TAM_MAX_MSG_BUFFER      64

#define CAPACIDADE_LISTA_1      100
#define CAPACIDADE_LISTA_2      200

#define NSEQ_MIN                1
#define NSEQ_MAX                99999

#define TIPO_DADOS_PROCESSO     1
#define TIPO_ESCALONAMENTO_PCP  2

// Temporizacoes (ms)
#define TEMPO_ETAPA1_MS         1000
#define PERIODO_SUPERVISORIO_MS 500
#define PERIODO_PCP_MIN_MS      1000
#define PERIODO_PCP_MAX_MS      5000

// Teclas de comando
#define TECLA_BLK_SUPERVISORIO  's'
#define TECLA_BLK_PCP           'm'
#define TECLA_BLK_RETIRADA      'r'
#define TECLA_BLK_EXIB_PROC     'p'
#define TECLA_BLK_GEST_PROD     'g'
#define TECLA_LIMPA_EXIB_PROC   '1'
#define TECLA_LIMPA_GEST_PROD   '2'
#define TECLA_ENCERRAR_ESC      27

// Eventos Win32
#define EV_RUN_SUPERVISORIO     "Local\\EvRun_Supervisorio"
#define EV_TERM_SUPERVISORIO    "Local\\EvTerm_Supervisorio"
#define EV_RUN_PCP              "Local\\EvRun_PCP"
#define EV_TERM_PCP             "Local\\EvTerm_PCP"
#define EV_RUN_RETIRADA         "Local\\EvRun_Retirada"
#define EV_TERM_RETIRADA        "Local\\EvTerm_Retirada"
#define EV_RUN_EXIB_PROC        "Local\\EvRun_ExibicaoProcesso"
#define EV_TERM_EXIB_PROC       "Local\\EvTerm_ExibicaoProcesso"
#define EV_RUN_GEST_PROD        "Local\\EvRun_GestaoProducao"
#define EV_TERM_GEST_PROD       "Local\\EvTerm_GestaoProducao"

// Semaforos Lista 1
#define SEM_L1_VAGAS            "Local\\SemLista1_Vagas"
#define SEM_L1_ITENS            "Local\\SemLista1_Itens"

// Objetos Lista 2 (Memoria Compartilhada)
#define SHM_LISTA_2             "Local\\ShmListaCircular2"
#define MTX_LISTA_2             "Local\\MtxListaCircular2"
#define SEM_L2_VAGAS            "Local\\SemLista2_Vagas"
#define SEM_L2_ITENS            "Local\\SemLista2_Itens"

// IPC - Mailslots
#define MAILSLOT_EXIB_PROC      "\\\\.\\mailslot\\SlotExibicaoProcesso"
#define MAILSLOT_GEST_PROD      "\\\\.\\mailslot\\SlotGestaoProducao"
#define CMD_IPC_LIMPAR_TELA     "CMD_CLS"

// Estruturas de Dados
struct ItemMensagem {
    int  tipo;
    char texto[TAM_MAX_MSG_BUFFER];
};

// No e estrutura da Lista Encadeada (Substitui ListaCircular1)
struct Node {
    ItemMensagem item;
    Node* next;
    Node(const ItemMensagem& msg) : item(msg), next(nullptr) {}
};

struct ListaEncadeada1 {
    Node* head;
    Node* tail;
    int count;
    ListaEncadeada1() : head(nullptr), tail(nullptr), count(0) {}
};

// Lista Circular 2 mantida como array (Ponteiros invalidam a memoria compartilhada entre processos)
struct ListaCircular2 {
    ItemMensagem buffer[CAPACIDADE_LISTA_2];
    int in;
    int out;
    int count;
};

struct ControleTarefa {
    HANDLE hEvRun;
    HANDLE hEvTerm;
    const char* nomeTarefa;
};

// Funcoes utilitarias
static inline bool AguardarPermissaoExecucaoOuTermino(HANDLE hEvRun, HANDLE hEvTerm, const char* nomeTarefa) {
    if (WaitForSingleObject(hEvTerm, 0) == WAIT_OBJECT_0) {
        return false;
    }

    if (WaitForSingleObject(hEvRun, 0) == WAIT_TIMEOUT) {
        printf("[%s] ESTADO: BLOQUEADA pelo comando do teclado. Aguardando liberacao ou ESC...\n", nomeTarefa);
        fflush(stdout);

        HANDLE hEventos[2] = { hEvTerm, hEvRun };
        DWORD resultado = WaitForMultipleObjects(2, hEventos, FALSE, INFINITE);

        if (resultado == WAIT_OBJECT_0) {
            printf("[%s] Notificacao de TERMINO (ESC) recebida durante bloqueio. Encerrando...\n", nomeTarefa);
            fflush(stdout);
            return false;
        }

        printf("[%s] ESTADO: DESBLOQUEADA. Retomando execucao normal.\n", nomeTarefa);
        fflush(stdout);
    }
    return true;
}

static inline void ObterTimestampAtual(char* bufferDestino, size_t tamanhoBuffer, bool incluirMilissegundos) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    if (incluirMilissegundos) {
        snprintf(bufferDestino, tamanhoBuffer, "%02d:%02d:%02d.%03d",
            st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    }
    else {
        snprintf(bufferDestino, tamanhoBuffer, "%02d:%02d:%02d",
            st.wHour, st.wMinute, st.wSecond);
    }
}

static inline void LimparTelaConsoleAtual(void) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) return;

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count, cellCount;
    COORD homeCoords = { 0, 0 };

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;
    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    FillConsoleOutputCharacter(hConsole, (TCHAR)' ', cellCount, homeCoords, &count);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cellCount, homeCoords, &count);
    SetConsoleCursorPosition(hConsole, homeCoords);
}

#endif // COMMON_H