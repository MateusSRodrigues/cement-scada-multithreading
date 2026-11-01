/*
 * =====================================================================================
 * Arquivo: common.h
 * Descricao: Cabecalho compartilhado (Contrato de Interface) entre os 3 processos
 *            da aplicacao (Principal, ExibicaoProcesso e GestaoProducao).
 * Disciplina: Automacao em Tempo Real (ELT127) - DELT/UFMG
 * =====================================================================================
 */

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

/* -------------------------------------------------------------------------------------
 * 1. CONSTANTES DE TAMANHO E LIMITES DO ENUNCIADO
 * ------------------------------------------------------------------------------------- */
// Tamanhos exatos das mensagens em caracteres (sem contar o '\0')
#define TAM_MSG_SUPERVISORIO    50
#define TAM_MSG_PCP             31
#define TAM_MAX_MSG_BUFFER      64   // Folga segura para alinhamento em memoria (50 + '\0' <= 64)

// Capacidades das Listas Circulares em Memoria RAM
#define CAPACIDADE_LISTA_1      100  // Lista entre Supervisorio/PCP e Retirada
#define CAPACIDADE_LISTA_2      200  // Lista entre Retirada e Exibicao de Processo

// Limites da sequencia de mensagens (NSEQ: 1 a 99999, voltando a 0 ou 1 conforme logica)
#define NSEQ_MIN                1
#define NSEQ_MAX                99999

// Tipos de mensagem
#define TIPO_DADOS_PROCESSO     1    // Gerada pelo Supervisorio (50 chars)
#define TIPO_ESCALONAMENTO_PCP  2    // Gerada pelo PCP (31 chars)

// Temporizacoes (em milissegundos)
#define TEMPO_ETAPA1_MS         1000 // Temporizacao provisoria da Etapa 1 com Sleep(1000)
#define PERIODO_SUPERVISORIO_MS 500  // Etapa 2: 500 ms fixos
#define PERIODO_PCP_MIN_MS      1000 // Etapa 2: 1 s minimo
#define PERIODO_PCP_MAX_MS      5000 // Etapa 2: 5 s maximo

/* -------------------------------------------------------------------------------------
 * 2. CODIGOS DE TECLAS DE COMANDO (Tarefa de Leitura do Teclado)
 * ------------------------------------------------------------------------------------- */
#define TECLA_BLK_SUPERVISORIO  's'  // Toggle bloqueio da leitura do Supervisorio
#define TECLA_BLK_PCP           'm'  // Toggle bloqueio da leitura do PCP
#define TECLA_BLK_RETIRADA      'r'  // Toggle bloqueio da retirada de mensagens
#define TECLA_BLK_EXIB_PROC     'p'  // Toggle bloqueio da exibicao de dados de processo
#define TECLA_BLK_GEST_PROD     'g'  // Toggle bloqueio da exibicao de gestao da producao
#define TECLA_LIMPA_EXIB_PROC   '1'  // Limpa console da exibicao de dados de processo
#define TECLA_LIMPA_GEST_PROD   '2'  // Limpa console da gestao da producao
#define TECLA_ENCERRAR_ESC      27   // Codigo ASCII da tecla ESC (0x1B)

/* -------------------------------------------------------------------------------------
 * 3. NOMES DOS OBJETOS NOMEADOS DO KERNEL WIN32 (Sincronismo entre Processos/Threads)
 * ------------------------------------------------------------------------------------- */
/*
 * Pares de Eventos para cada uma das 5 tarefas controladas pelo teclado:
 * - Evento de Execucao/Bloqueio (EV_RUN_*): Criado com bManualReset = TRUE e bInitialState = TRUE.
 *   Quando ativo (TRUE), a tarefa roda livremente. Quando bloqueado pela tecla (ResetEvent),
 *   a tarefa congela no WaitForMultipleObjects.
 * - Evento de Termino (EV_TERM_*): Criado com bManualReset = TRUE e bInitialState = FALSE.
 *   Quando ESC e pressionado, SetEvent() acorda imediatamente qualquer tarefa, mesmo bloqueada.
 */
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

/* Semaforos da Lista Circular 1 (no Processo Principal) */
#define SEM_L1_VAGAS            "Local\\SemLista1_Vagas"
#define SEM_L1_ITENS            "Local\\SemLista1_Itens"

/* Objetos da Lista Circular 2 em Memoria Compartilhada (Etapa 2: Principal <-> ExibicaoProcesso) */
#define SHM_LISTA_2             "Local\\ShmListaCircular2"
#define MTX_LISTA_2             "Local\\MtxListaCircular2"
#define SEM_L2_VAGAS            "Local\\SemLista2_Vagas"
#define SEM_L2_ITENS            "Local\\SemLista2_Itens"

/* -------------------------------------------------------------------------------------
 * 4. CANAIS DE IPC - MAILSLOTS (Etapa 2)
 * ------------------------------------------------------------------------------------- */
// Mailslot do processo ExibicaoProcesso (recebe comando de limpeza da tecla '1')
#define MAILSLOT_EXIB_PROC      "\\\\.\\mailslot\\SlotExibicaoProcesso"

// Mailslot do processo GestaoProducao (recebe msgs Tipo 2 da Retirada + limpeza da tecla '2')
#define MAILSLOT_GEST_PROD      "\\\\.\\mailslot\\SlotGestaoProducao"

// Comando padronizado enviado via IPC para limpeza de tela
#define CMD_IPC_LIMPAR_TELA     "CMD_CLS"

/* -------------------------------------------------------------------------------------
 * 5. ESTRUTURAS DE DADOS E LISTAS CIRCULARES
 * ------------------------------------------------------------------------------------- */

/* Estrutura de um item armazenado na Lista Circular 1 ou 2 */
typedef struct {
    int  tipo;                          // 1 = Dados de Processo (50 chars), 2 = PCP (31 chars)
    char texto[TAM_MAX_MSG_BUFFER];     // String formatada terminada em '\0'
} ItemMensagem;

/*
 * Estrutura da Lista Circular 1 (100 posicoes - Memoria RAM local do Processo Principal)
 * Nota: O mutex de exclusao mutua dos indices pode ser pthread_mutex_t (para o bonus Pthreads)
 * declarado no modulo da Lista 1, enquanto os semaforos Win32 contam vagas e itens.
 */
typedef struct {
    ItemMensagem buffer[CAPACIDADE_LISTA_1];
    int in;                             // Indice de escrita (Produtores: Supervisorio e PCP)
    int out;                            // Indice de leitura (Consumidor: Retirada)
    int count;                          // Total de mensagens atualmente na lista
} ListaCircular1;

/*
 * Estrutura da Lista Circular 2 (200 posicoes - Memoria Compartilhada na Etapa 2)
 * Compartilhada em RAM entre a Tarefa de Retirada e a Tarefa de Exibicao de Dados de Processo.
 */
typedef struct {
    ItemMensagem buffer[CAPACIDADE_LISTA_2];
    int in;                             // Indice de escrita (Produtor: Retirada)
    int out;                            // Indice de leitura (Consumidor: Exibicao de Processo)
    int count;                          // Total de mensagens atualmente na lista
} ListaCircular2;

/* Estrutura utilitaria para agrupar o par de eventos de controle de uma tarefa */
typedef struct {
    HANDLE hEvRun;                      // Evento de bloqueio/desbloqueio (on/off)
    HANDLE hEvTerm;                     // Evento de encerramento (ESC)
    const char* nomeTarefa;             // Nome amigavel para logs na console
} ControleTarefa;

/* -------------------------------------------------------------------------------------
 * 6. FUNCOES INLINE UTILITARIAS COMPARTILHADAS
 * ------------------------------------------------------------------------------------- */

/*
 * Funcao: AguardarPermissaoExecucaoOuTermino
 * Descricao: Verifica se a tarefa esta bloqueada pelo teclado ou se recebeu comando ESC.
 *            Se estiver bloqueada, imprime o aviso na console correspondente e aguarda
 *            ate ser desbloqueada OU ate que ESC seja pressionado.
 * Retorno:   true  -> Tarefa liberada para executar mais um ciclo.
 *            false -> Comando ESC recebido! A tarefa deve liberar recursos e encerrar.
 */
static inline bool AguardarPermissaoExecucaoOuTermino(HANDLE hEvRun, HANDLE hEvTerm, const char* nomeTarefa) {
    // 1. Verifica instantaneamente se o ESC ja foi acionado
    if (WaitForSingleObject(hEvTerm, 0) == WAIT_OBJECT_0) {
        return false;
    }

    // 2. Verifica sem bloquear se o evento de execucao esta pausado (ResetEvent)
    if (WaitForSingleObject(hEvRun, 0) == WAIT_TIMEOUT) {
        printf("[%s] ESTADO: BLOQUEADA pelo comando do teclado. Aguardando liberacao ou ESC...\n", nomeTarefa);
        fflush(stdout);

        // Espera ate que seja desbloqueada (hEvRun) OU que o usuario aperte ESC (hEvTerm)
        HANDLE hEventos[2] = { hEvTerm, hEvRun };
        DWORD resultado = WaitForMultipleObjects(2, hEventos, FALSE, INFINITE);

        if (resultado == WAIT_OBJECT_0) {
            // hEvTerm (indice 0) disparou enquanto a tarefa estava bloqueada!
            printf("[%s] Notificacao de TERMINO (ESC) recebida durante bloqueio. Encerrando...\n", nomeTarefa);
            fflush(stdout);
            return false;
        }

        printf("[%s] ESTADO: DESBLOQUEADA. Retomando execucao normal.\n", nomeTarefa);
        fflush(stdout);
    }

    return true;
}

/*
 * Funcao: ObterTimestampAtual
 * Descricao: Preenche o buffer com a hora corrente do sistema no formato HH:MM:SS.MSS
 *            (12 caracteres) ou HH:MM:SS (8 caracteres), conforme exigido pelo enunciado.
 */
static inline void ObterTimestampAtual(char* bufferDestino, size_t tamanhoBuffer, bool incluirMilissegundos) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    if (incluirMilissegundos) {
        snprintf(bufferDestino, tamanhoBuffer, "%02d:%02d:%02d.%03d",
                 st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    } else {
        snprintf(bufferDestino, tamanhoBuffer, "%02d:%02d:%02d",
                 st.wHour, st.wMinute, st.wSecond);
    }
}

/*
 * Funcao: LimparTelaConsoleAtual
 * Descricao: Limpa a janela de console do processo atual usando a API Win32 de Console.
 */
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

#endif /* COMMON_H */