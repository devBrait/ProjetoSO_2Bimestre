#include <stdio.h>
#include <stdlib.h>

#define TAMANHO_PROCESSO 16384

// Definição das Estruturas
typedef struct {
    int presente;        // 1 se a página está na memória, 0 caso contrário
    int frame;           // Número do frame onde a página está alocada (-1 se não alocada)
    int modificada;      // 1 se a página foi modificada, 0 caso contrário
    int referenciada;    // 1 se a página foi referenciada recentemente, 0 caso contrário
    int tempo_carga;     // Instante em que a página foi carregada na memória
    int ultimo_acesso;   // Instante do último acesso à página
} Pagina;

typedef struct {
    int pid;             // Identificador do processo
    int tamanho;         // Tamanho do processo em bytes
    int num_paginas;     // Número de páginas do processo
    Pagina *tabela_paginas; // Tabela de páginas do processo
} Processo;

typedef struct {
    int num_frames;      // Número total de frames na memória física
    int *frames;         // Array de frames (cada elemento contém o pid e a página)
                         // Ex: frames[i] = (pid << 16) | num_pagina
    int *tempo_carga;    // Tempo em que cada frame foi carregado (para FIFO)
    int ponteiro_clock;  // Ponteiro para o algoritmo CLOCK (não usado neste projeto)
} MemoriaFisica;

typedef struct {
    int tempo_atual;     // Contador de tempo da simulação
    int tamanho_pagina;  // Tamanho da página em bytes
    int tamanho_memoria_fisica; // Tamanho da memória física em bytes
    int num_processos;   // Número de processos na simulação
    Processo *processos; // Array de processos
    MemoriaFisica memoria; // Memória física
    // Estatísticas
    int total_acessos;   // Total de acessos à memória
    int page_faults;     // Total de page faults ocorridos
    // Algoritmo de substituição atual
    int algoritmo;       // 0=FIFO, 1=LRU, 2=CLOCK, 3=RANDOM, 4=CUSTOM
} Simulador;


void exibe_menu();

// Inicializa o simulador com os parâmetros fornecidos
Simulador* inicializar_simulador(int tamanho_pagina , int tamanho_memoria_fisica, int algoritmo_selecionado);

// Cria um novo processo e o adiciona ao simulador
Processo* criar_processo ( Simulador *sim , int tamanho_processo );

// Menu principal
void exibe_menu() {
    int opcao = -1;

    while (1) {
        printf("===== SIMULADOR DE PAGINAÇÃO =====\n");
        printf("Escolha o algoritmo de substituição de páginas:\n");
        printf("1 - FIFO\n");
        printf("2 - LRU\n");
        printf("3 - Sair\n");
        printf("Opção: ");
        scanf("%d", &opcao);

        if (opcao == 3) {
            printf("Muito obrigado por utilizar o simulador, espero que tenha gostado!!!\n");
            break;
        }

        if (opcao != 1 && opcao != 2) {
            printf("Opção inválida! Tente novamente.\n\n");
            continue;
        }

        int algoritmo = (opcao == 1) ? 0 : 1;

        Simulador *sim = inicializar_simulador(4096, 16384, algoritmo);

        printf("\nSimulador inicializado com sucesso!\n");
        printf("Algoritmo selecionado: %s\n\n", (algoritmo == 0) ? "FIFO" : "LRU");

        // Liberação de memória
        free(sim->memoria.frames);
        free(sim->memoria.tempo_carga);
        free(sim->processos);
        free(sim);

        printf("\n===============================\n\n");
    }
}


// Inicializa o simulador
Simulador* inicializar_simulador(int tamanho_pagina, int tamanho_memoria, int algoritmo_selecionado) {
    Simulador *sim = (Simulador*) malloc(sizeof(Simulador));
    sim->tempo_atual = 0;
    sim->tamanho_pagina = tamanho_pagina;
    sim->tamanho_memoria_fisica = tamanho_memoria;
    sim->num_processos = 0;
    sim->processos = NULL;

    sim->memoria.num_frames = tamanho_memoria / tamanho_pagina;
    sim->memoria.frames = (int*) malloc(sim->memoria.num_frames * sizeof(int));
    sim->memoria.tempo_carga = (int*) malloc(sim->memoria.num_frames * sizeof(int));

    for (int i = 0; i < sim->memoria.num_frames; i++) {
        sim->memoria.frames[i] = -1; // -1 indica frame vazio
        sim->memoria.tempo_carga[i] = -1;
    }

    sim->total_acessos = 0;
    sim->page_faults = 0;
    sim->algoritmo = algoritmo_selecionado;

    for (int i = 0; i < 3; i++) {
        criar_processo(sim, TAMANHO_PROCESSO);
    }

    return sim;
}

Processo* criar_processo(Simulador *sim, int tamanho_processo) {
    Processo *novo = (Processo*) malloc(sizeof(Processo));

    novo->pid = sim->num_processos + 1;
    novo->tamanho = tamanho_processo;
    novo->num_paginas = tamanho_processo / sim->tamanho_pagina;

    if (tamanho_processo % sim->tamanho_pagina != 0) {
        novo->num_paginas += 1;
    }

    novo->tabela_paginas = (Pagina*) malloc(novo->num_paginas * sizeof(Pagina));

    for (int i = 0; i < novo->num_paginas; i++) {
        novo->tabela_paginas[i].presente = 0;
        novo->tabela_paginas[i].frame = -1;
        novo->tabela_paginas[i].modificada = 0;
        novo->tabela_paginas[i].referenciada = 0;
        novo->tabela_paginas[i].tempo_carga = -1;
        novo->tabela_paginas[i].ultimo_acesso = -1;
    }

    sim->num_processos++;
    sim->processos = (Processo*) realloc(sim->processos, sim->num_processos * sizeof(Processo));
    sim->processos[sim->num_processos - 1] = *novo;

    printf("Processo PID %d criado com %d páginas.\n",
           novo->pid, novo->num_paginas);

    free(novo);  // Liberação do ponteiro 

    return &sim->processos[sim->num_processos - 1];
}

int main() {
    exibe_menu();
    return 0;
}