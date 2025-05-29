#include <stdio.h>
#include <stdlib.h>

#define TAMANHO_PAGINA 4096
#define TAMANHO_MEMORIA 16384

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

// Exibição do menu
void exibe_menu();

// Inicializa o simulador com os parâmetros fornecidos
Simulador* inicializar_simulador(int tamanho_pagina, int tamanho_memoria_fisica, int algoritmo_selecionado);

// Cria um novo processo e o adiciona ao simulador
Processo* criar_processo(Simulador *sim, int tamanho_processo);

// Traduz um endereço virtual para físico
int traduzir_endereco(Simulador *sim, int pid, int endereco_virtual);
// Extrai o número da página e o deslocamento de um endereço virtual
void extrair_pagina_deslocamento(Simulador *sim, int endereco_virtual, int *pagina , int * deslocamento );

// Verifica se uma página está presente na memória física
int verificar_pagina_presente(Simulador *sim , int pid , int pagina);
// Carrega uma página na memória física
int carregar_pagina(Simulador *sim , int pid , int pagina);
// Implementa o algoritmo de substituição de páginas FIFO
int substituir_pagina_fifo(Simulador *sim);
// Implementa o algoritmo de substituição de páginas LRU
int substituir_pagina_lru(Simulador *sim);

// Exibe o estado atual da memória física
void exibir_memoria_fisica(Simulador *sim);
// Exibe estatísticas da simulação
void exibir_estatisticas(Simulador *sim);
// Registra um acesso à memória
void registrar_acesso(Simulador *sim, int pid, int pagina, int tipo_acesso);

// Executa a simulação com uma sequência de acessos à memória
void executar_simulacao(Simulador *sim, int algoritmo);
// Simula um acesso à memória
int acessar_memoria(Simulador *sim, int pid, int endereco_virtual);


void extrair_pagina_deslocamento(Simulador *sim, int endereco_virtual, int *pagina, int *deslocamento) {
    *pagina = endereco_virtual / sim->tamanho_pagina;
    *deslocamento = endereco_virtual % sim->tamanho_pagina;
}

int verificar_pagina_presente(Simulador *sim, int pid, int pagina) {
    Processo *proc = &sim->processos[pid - 1];
    return proc->tabela_paginas[pagina].presente;
}

int substituir_pagina_fifo(Simulador *sim) {
    int mais_antigo = 0;
    int tempo_min = sim->tempo_atual + 1;

    for (int i = 0; i < sim->memoria.num_frames; i++) {
        if (sim->memoria.tempo_carga[i] < tempo_min) {
            tempo_min = sim->memoria.tempo_carga[i];
            mais_antigo = i;
        }
    }

    return mais_antigo;
}

int substituir_pagina_lru(Simulador *sim) {
    int frame_lru = 0;
    int tempo_min = sim->tempo_atual + 1; // Inicializa com valor maior que qualquer tempo possível

    for (int i = 0; i < sim->memoria.num_frames; i++) {
        int val = sim->memoria.frames[i];
        int pid = val >> 16;
        int pagina = val & 0xFFFF;

        Processo *proc = &sim->processos[pid - 1];
        int ultimo_acesso = proc->tabela_paginas[pagina].ultimo_acesso;
        
        // Debug: imprimir informações para análise
        printf("Frame %d: P%d-Pag%d, ultimo_acesso=%d\n", i, pid, pagina, ultimo_acesso);

        if (ultimo_acesso < tempo_min) {
            tempo_min = ultimo_acesso;
            frame_lru = i;
        }
    }

    printf("LRU escolheu frame %d (tempo_min=%d)\n", frame_lru, tempo_min);
    return frame_lru;
}


int carregar_pagina(Simulador *sim, int pid, int pagina) {
    Processo *proc = &sim->processos[pid - 1];
    Pagina *pag = &proc->tabela_paginas[pagina];

    // Verifica se há algum frame livre
    for (int i = 0; i < sim->memoria.num_frames; i++) {
        if (sim->memoria.frames[i] == -1) {
            sim->memoria.frames[i] = (pid << 16) | pagina;
            sim->memoria.tempo_carga[i] = sim->tempo_atual;
            pag->presente = 1;
            pag->frame = i;
            pag->tempo_carga = sim->tempo_atual;
            pag->ultimo_acesso = sim->tempo_atual; 
            return i;
        }
    }

    int frame_substituido;
    const char *nome_algoritmo;

    if (sim->algoritmo == 0) {
        frame_substituido = substituir_pagina_fifo(sim);
        nome_algoritmo = "FIFO";
    } else if (sim->algoritmo == 1) {
        frame_substituido = substituir_pagina_lru(sim);
        nome_algoritmo = "LRU";
    } else {
        printf("Erro: algoritmo de substituição não implementado.\n");
        exit(1);
    }

    // Recupera a página antiga a ser substituída
    int pid_antigo = sim->memoria.frames[frame_substituido] >> 16;
    int pagina_antiga = sim->memoria.frames[frame_substituido] & 0xFFFF;

    Processo *proc_antigo = &sim->processos[pid_antigo - 1];
    proc_antigo->tabela_paginas[pagina_antiga].presente = 0;
    proc_antigo->tabela_paginas[pagina_antiga].frame = -1;

    // Exibe a substituição
    printf("Tempo t=%d: Substituindo Página %d do Processo %d no Frame %d pela Página %d do Processo %d (%s)\n",
           sim->tempo_atual, pagina_antiga, pid_antigo, frame_substituido, pagina, pid, nome_algoritmo);

    // Atualiza a nova página
    sim->memoria.frames[frame_substituido] = (pid << 16) | pagina;
    sim->memoria.tempo_carga[frame_substituido] = sim->tempo_atual;

    pag->presente = 1;
    pag->frame = frame_substituido;
    pag->tempo_carga = sim->tempo_atual;
    pag->ultimo_acesso = sim->tempo_atual;

    return frame_substituido;
}


int traduzir_endereco(Simulador *sim, int pid, int endereco_virtual) {
    int pagina, deslocamento;
    extrair_pagina_deslocamento(sim, endereco_virtual, &pagina, &deslocamento);
    Processo *proc = &sim->processos[pid - 1];

    if (pagina >= proc->num_paginas) {
        printf("Erro: Acesso a página inválida.\n");
        return -1;
    }

    if (!verificar_pagina_presente(sim, pid, pagina)) {
        printf("Tempo t=%d: [PAGE FAULT] Página %d do Processo %d não está na memória física!\n", sim->tempo_atual, pagina, pid);
        sim->page_faults++;
        carregar_pagina(sim, pid, pagina);
    }else {
        Pagina *pag = &proc->tabela_paginas[pagina];
        pag->ultimo_acesso = sim->tempo_atual;
    }

    Pagina *pag = &proc->tabela_paginas[pagina];
    pag->ultimo_acesso = sim->tempo_atual;
    sim->total_acessos++;

    int endereco_fisico = (pag->frame * sim->tamanho_pagina) + deslocamento;

    printf("Tempo t=%d: Endereço Virtual (P%d): %d -> Página: %d -> Frame: %d -> Endereço Físico: %d\n",
           sim->tempo_atual, pid, endereco_virtual, pagina, pag->frame, endereco_fisico);

    return endereco_fisico;
}

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
    printf("--------\n");
}

void exibir_estatisticas(Simulador *sim) {
    printf("\n======== ESTATÍSTICAS DA SIMULAÇÃO ========\n");
    printf("Total de acessos à memória: %d\n", sim->total_acessos);
    printf("Total de page faults: %d\n", sim->page_faults);
    float taxa = (sim->total_acessos == 0) ? 0 : ((float)sim->page_faults / sim->total_acessos) * 100;
    printf("Taxa de page faults: %.2f%%\n", taxa);
}

int acessar_memoria(Simulador *sim, int pid, int endereco_virtual) {
    int endereco_fisico = traduzir_endereco(sim, pid, endereco_virtual);
    exibir_memoria_fisica(sim);
    sim->tempo_atual++;
    return endereco_fisico;
}

void executar_simulacao(Simulador *sim, int algoritmo) {
    
    int acessos[][2] = {
    {1, 1111}, {2, 2222}, {3, 3333},
    {1, 4444}, {2, 1234}, {3, 2345},
    {1, 4096}, {1, 8192}, {2, 4096}, {3, 8192},
    {1, 1024}, {2, 2048}, {3, 3072}, {1, 512}, {2, 6000},
    {3, 999}, {1, 3500}, {2, 7000}, {3, 1200}, {1, 8192}
    };

    int total = sizeof(acessos) / sizeof(acessos[0]);
    
    printf("\n=== INICIANDO SIMULAÇÃO COM %s ===\n", 
           (algoritmo == 0) ? "FIFO" : "LRU");
    
    for (int i = 0; i < total; i++) {
        printf("\n--- Acesso %d: P%d, Endereço %d ---\n", 
               i+1, acessos[i][0], acessos[i][1]);
        acessar_memoria(sim, acessos[i][0], acessos[i][1]);
    }

    exibir_estatisticas(sim);
}

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
        sim->memoria.frames[i] = -1;
        sim->memoria.tempo_carga[i] = -1;
    }

    sim->total_acessos = 0;
    sim->page_faults = 0;
    sim->algoritmo = algoritmo_selecionado;

    for (int i = 0; i < 3; i++) {
        criar_processo(sim, TAMANHO_MEMORIA);
    }

    return sim;
}

Processo* criar_processo(Simulador *sim, int tamanho_processo) {
    sim->num_processos++;
    sim->processos = (Processo*) realloc(sim->processos, sim->num_processos * sizeof(Processo));
    
    Processo *novo = &sim->processos[sim->num_processos - 1]; // Usar diretamente o espaço do array

    novo->pid = sim->num_processos;
    novo->tamanho = tamanho_processo;
    novo->num_paginas = tamanho_processo / sim->tamanho_pagina;
    if (tamanho_processo % sim->tamanho_pagina != 0)
        novo->num_paginas++;

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

// Exibição do menu
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
        fgets(linha, sizeof(linha), stdin);// limpa buffer

        if (opcao == 3) {
            printf("Muito obrigado por utilizar o simulador, espero que tenha gostado!!!\n");
            break;
        }

        if (opcao != 1 && opcao != 2) {
            printf("Opção inválida! Tente novamente.\n\n");
            continue;
        }

        int algoritmo = (opcao == 1) ? 0 : 1;
        
        Simulador *sim = inicializar_simulador(TAMANHO_PAGINA, TAMANHO_MEMORIA, algoritmo);
        printf("\nSimulador inicializado com sucesso!\n");
        printf("Algoritmo selecionado: %s\n", (algoritmo == 0) ? "FIFO" : "LRU");
        printf("Memória física: %d bytes (%d frames)\n", 
               TAMANHO_MEMORIA, TAMANHO_MEMORIA/TAMANHO_PAGINA);
        printf("Processos: 3 processos, cada um com %d páginas\n\n", 
               TAMANHO_MEMORIA/TAMANHO_PAGINA);

        executar_simulacao(sim, algoritmo);

        free(sim->memoria.frames);
        free(sim->memoria.tempo_carga);

        for (int i = 0; i < sim->num_processos; i++) {
            free(sim->processos[i].tabela_paginas);
        }

        free(sim->processos);
        free(sim);

        printf("\n===============================\n\n");
    }
}

int main() {
    exibe_menu();
    return 0;
}