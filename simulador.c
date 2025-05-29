#include <stdio.h>
#include <stdlib.h>
#include "simulador.h"
#include "memoria.h"

// Função que inicializa o simulador de memória
Simulador* inicializar_simulador(int tamanho_pagina, int tamanho_memoria, int algoritmo_selecionado) {
    Simulador *sim = (Simulador*) malloc(sizeof(Simulador));

    // Parâmetros básicos
    sim->tempo_atual = 0;
    sim->tamanho_pagina = tamanho_pagina;
    sim->tamanho_memoria_fisica = tamanho_memoria;
    sim->num_processos = 0;
    sim->processos = NULL;

    // Calcula o número de frames na memória física
    sim->memoria.num_frames = tamanho_memoria / tamanho_pagina;
    sim->memoria.frames = (int*) malloc(sim->memoria.num_frames * sizeof(int));
    sim->memoria.tempo_carga = (int*) malloc(sim->memoria.num_frames * sizeof(int));

    // Inicializa todos os frames como vazios
    for (int i = 0; i < sim->memoria.num_frames; i++) {
        sim->memoria.frames[i] = -1;
        sim->memoria.tempo_carga[i] = -1;
    }

    // Estatísticas
    sim->total_acessos = 0;
    sim->page_faults = 0;
    sim->algoritmo = algoritmo_selecionado;

    // Cria 3 processos simulados
    for (int i = 0; i < 3; i++) {
        criar_processo(sim, TAMANHO_MEMORIA);
    }

    return sim;
}

// Função que cria um processo e sua tabela de páginas
Processo* criar_processo(Simulador *sim, int tamanho_processo) {
    sim->num_processos++;

    sim->processos = (Processo*) realloc(sim->processos, sim->num_processos * sizeof(Processo));

    Processo *novo = &sim->processos[sim->num_processos - 1];

    // Dados do processo
    novo->pid = sim->num_processos;
    novo->tamanho = tamanho_processo;
    novo->num_paginas = tamanho_processo / sim->tamanho_pagina;
    if (tamanho_processo % sim->tamanho_pagina != 0)
        novo->num_paginas++;

    // Inicializa tabela de páginas
    novo->tabela_paginas = (Pagina*) malloc(novo->num_paginas * sizeof(Pagina));
    for (int i = 0; i < novo->num_paginas; i++) {
        novo->tabela_paginas[i].presente = 0;
        novo->tabela_paginas[i].frame = -1;
        novo->tabela_paginas[i].modificada = 0;
        novo->tabela_paginas[i].referenciada = 0;
        novo->tabela_paginas[i].tempo_carga = -1;
        novo->tabela_paginas[i].ultimo_acesso = -1;
    }

    return novo;
}

// Exibe o estado atual da memória física
void exibir_memoria_fisica(Simulador *sim) {
    printf("Estado da Memória Física:\n");
    
    for (int i = 0; i < sim->memoria.num_frames; i++) {
        int val = sim->memoria.frames[i];
        if (val == -1) {
            printf("--------\n| ---- |\n");
        } else {
            int pid = val >> 16;
            int pagina = val & 0xFFFF;
            printf("--------\n| P%d-%d |\n", pid, pagina); 
        }
    }
    printf("--------\n\n");
}

// Exibe estatísticas da simulação
void exibir_estatisticas(Simulador *sim) {
    printf("\n======== ESTATÍSTICAS DA SIMULAÇÃO ========\n");
    printf("Total de acessos à memória: %d\n", sim->total_acessos);
    printf("Total de page faults: %d\n", sim->page_faults);
    float taxa = (sim->total_acessos == 0) ? 0 : ((float)sim->page_faults / sim->total_acessos) * 100;
    printf("Taxa de page faults: %.2f%%\n\n", taxa);
}

// Executa uma sequência fixa de acessos à memória
void executar_simulacao(Simulador *sim, int algoritmo) {
    // Acessos simulados: processo, endereço_virtual
    int acessos[][2] = {
        {1, 1111}, {2, 2222}, {3, 3333},
        {1, 4444}, {2, 1234}, {3, 2345},
        {1, 4096}, {1, 8192}, {2, 4096}, {3, 8192},
        {1, 1024}, {2, 2048}, {3, 3072}, {1, 512}, {2, 6000},
        {3, 999}, {1, 3500}, {2, 7000}, {3, 1200}, {1, 8192}
    };

    int total = sizeof(acessos) / sizeof(acessos[0]);

    for (int i = 0; i < total; i++) {
        acessar_memoria(sim, acessos[i][0], acessos[i][1]);
        exibir_memoria_fisica(sim);
    }

    // Exibe as estatísticas finais
    exibir_estatisticas(sim);
}

// Função principal do menu da simulação
void exibe_menu() {
    int opcao = -1;
    printf("===== SIMULADOR DE PAGINAÇÃO =====\n");

    while (1) {
        printf("Escolha o algoritmo de substituição de páginas:\n");
        printf("1 - FIFO\n");
        printf("2 - LRU\n");
        printf("3 - Sair\n");
        printf("Opção: ");
        scanf("%d", &opcao);


        char linha[100];
        fgets(linha, sizeof(linha), stdin);

        if (opcao == 3) {
            printf("\nMuito obrigado por utilizar o simulador, espero que tenha gostado!!!\n");
            break;
        }

        if (opcao != 1 && opcao != 2) {
            printf("Opção inválida! Tente novamente.\n\n");
            continue;
        }

        int algoritmo = (opcao == 1) ? 0 : 1;

        Simulador *sim = inicializar_simulador(TAMANHO_PAGINA, TAMANHO_MEMORIA, algoritmo);

        // Informações iniciais
        printf("\n=== Simulador inicializado com sucesso ===\n");
        printf("Tamanho da página: %d bytes (%d KB)\n", TAMANHO_PAGINA, TAMANHO_PAGINA/1024);
        printf("Tamanho da memória física: %d bytes (%d KB)\n", 
               TAMANHO_MEMORIA, TAMANHO_MEMORIA/1024);
        printf("Número de frames: %d\n", TAMANHO_MEMORIA / TAMANHO_PAGINA);
        printf("Algoritmo selecionado: %s\n\n", (algoritmo == 0) ? "FIFO" : "LRU");

        executar_simulacao(sim, algoritmo);

        free(sim->memoria.frames);
        free(sim->memoria.tempo_carga);
        for (int i = 0; i < sim->num_processos; i++) {
            free(sim->processos[i].tabela_paginas);
        }
        free(sim->processos);
        free(sim);
    }
}
