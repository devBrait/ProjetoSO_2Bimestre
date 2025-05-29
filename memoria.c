#include <stdio.h>
#include <stdlib.h>
#include "memoria.h"

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

        if (ultimo_acesso < tempo_min) {
            tempo_min = ultimo_acesso;
            frame_lru = i;
        }
    }

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
    } else {
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

int acessar_memoria(Simulador *sim, int pid, int endereco_virtual) {
    int endereco_fisico = traduzir_endereco(sim, pid, endereco_virtual);
    sim->tempo_atual++;
    return endereco_fisico;
}