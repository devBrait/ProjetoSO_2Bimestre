#include <stdio.h>
#include <stdlib.h>
#include "memoria.h"

// Extrai o número da página e o deslocamento a partir de um endereço virtual
void extrair_pagina_deslocamento(Simulador *sim, int endereco_virtual, int *pagina, int *deslocamento) {
    *pagina = endereco_virtual / sim->tamanho_pagina;
    *deslocamento = endereco_virtual % sim->tamanho_pagina;
}

// Verifica se uma página de um processo está presente na memória física
int verificar_pagina_presente(Simulador *sim, int pid, int pagina) {
    Processo *proc = &sim->processos[pid - 1];
    return proc->tabela_paginas[pagina].presente;
}

// Algoritmo de substituição de página: FIFO (First-In, First-Out)
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

// Algoritmo de substituição de página: LRU (Least Recently Used)
int substituir_pagina_lru(Simulador *sim) {
    int frame_lru = 0;
    int tempo_min = sim->tempo_atual + 1;

    for (int i = 0; i < sim->memoria.num_frames; i++) {
        int val = sim->memoria.frames[i];
        int pid = val >> 16;             // Extrai o PID (16 bits mais altos)
        int pagina = val & 0xFFFF;       // Extrai o número da página (16 bits mais baixos)

        Processo *proc = &sim->processos[pid - 1];
        int ultimo_acesso = proc->tabela_paginas[pagina].ultimo_acesso;

        if (ultimo_acesso < tempo_min) {
            tempo_min = ultimo_acesso;
            frame_lru = i;
        }
    }

    return frame_lru;
}

// Carrega uma página para a memória, substituindo caso necessário
int carregar_pagina(Simulador *sim, int pid, int pagina) {
    Processo *proc = &sim->processos[pid - 1];
    Pagina *pag = &proc->tabela_paginas[pagina];

    // Verifica se há um frame livre
    for (int i = 0; i < sim->memoria.num_frames; i++) {
        if (sim->memoria.frames[i] == -1) {  // Frame livre
            sim->memoria.frames[i] = (pid << 16) | pagina;
            sim->memoria.tempo_carga[i] = sim->tempo_atual;
            pag->presente = 1;
            pag->frame = i;
            pag->tempo_carga = sim->tempo_atual;
            pag->ultimo_acesso = sim->tempo_atual;
            return i;
        }
    }

    // Caso não houver frame livre
    int frame_substituido;
    const char *nome_algoritmo;

    if (sim->algoritmo == 0) {  // FIFO
        frame_substituido = substituir_pagina_fifo(sim);
        nome_algoritmo = "FIFO";
    } else if (sim->algoritmo == 1) {  // LRU
        frame_substituido = substituir_pagina_lru(sim);
        nome_algoritmo = "LRU";
    } else {
        printf("Erro: algoritmo de substituição não implementado.\n");
        exit(1);
    }

    // Desaloca a página antiga que será substituída
    int pid_antigo = sim->memoria.frames[frame_substituido] >> 16;
    int pagina_antiga = sim->memoria.frames[frame_substituido] & 0xFFFF;

    Processo *proc_antigo = &sim->processos[pid_antigo - 1];
    proc_antigo->tabela_paginas[pagina_antiga].presente = 0;
    proc_antigo->tabela_paginas[pagina_antiga].frame = -1;

    // Mensagem informando a substituição
    printf("Tempo t=%d: Substituindo Página %d do Processo %d no Frame %d pela Página %d do Processo %d (%s)\n",
           sim->tempo_atual, pagina_antiga, pid_antigo, frame_substituido, pagina, pid, nome_algoritmo);

    // Carrega a nova página no frame substituído
    sim->memoria.frames[frame_substituido] = (pid << 16) | pagina;
    sim->memoria.tempo_carga[frame_substituido] = sim->tempo_atual;

    pag->presente = 1;
    pag->frame = frame_substituido;
    pag->tempo_carga = sim->tempo_atual;
    pag->ultimo_acesso = sim->tempo_atual;

    return frame_substituido;
}

// Traduz um endereço virtual em endereço físico
int traduzir_endereco(Simulador *sim, int pid, int endereco_virtual) {
    int pagina, deslocamento;
    extrair_pagina_deslocamento(sim, endereco_virtual, &pagina, &deslocamento);
    Processo *proc = &sim->processos[pid - 1];

    // Verifica se a página é válida
    if (pagina >= proc->num_paginas) {
        printf("Erro: Acesso a página inválida.\n");
        return -1;
    }

    // Verifica se a página está na memória
    if (!verificar_pagina_presente(sim, pid, pagina)) {
        printf("Tempo t=%d: [PAGE FAULT] Página %d do Processo %d não está na memória física!\n", 
               sim->tempo_atual, pagina, pid);
        sim->page_faults++;
        carregar_pagina(sim, pid, pagina);  // Carrega a página
    } else {
        Pagina *pag = &proc->tabela_paginas[pagina];
        pag->ultimo_acesso = sim->tempo_atual;
    }

    // Incrementa estatísticas e calcula o endereço físico
    Pagina *pag = &proc->tabela_paginas[pagina];
    pag->ultimo_acesso = sim->tempo_atual;
    sim->total_acessos++;

    int endereco_fisico = (pag->frame * sim->tamanho_pagina) + deslocamento;

    printf("Tempo t=%d: Endereço Virtual (P%d): %d -> Página: %d -> Frame: %d -> Endereço Físico: %d\n",
           sim->tempo_atual, pid, endereco_virtual, pagina, pag->frame, endereco_fisico);

    return endereco_fisico;
}

// Acessa a memória, traduzindo o endereço e avançando o tempo do simulador
int acessar_memoria(Simulador *sim, int pid, int endereco_virtual) {
    int endereco_fisico = traduzir_endereco(sim, pid, endereco_virtual);
    sim->tempo_atual++;  // Avança o tempo a cada acesso
    return endereco_fisico;
}
