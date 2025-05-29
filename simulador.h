#ifndef SIMULADOR_H
#define SIMULADOR_H

#include "estruturas.h"

// Inicialização e gerenciamento do simulador
Simulador* inicializar_simulador(int tamanho_pagina, int tamanho_memoria_fisica, int algoritmo_selecionado);
Processo* criar_processo(Simulador *sim, int tamanho_processo);

// Inicialização de exibição e interface
void exibe_menu();
void exibir_memoria_fisica(Simulador *sim);
void exibir_estatisticas(Simulador *sim);

// Inicialização de execução
void executar_simulacao(Simulador *sim, int algoritmo);

#endif