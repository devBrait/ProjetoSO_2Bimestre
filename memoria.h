#ifndef MEMORIA_H
#define MEMORIA_H

#include "estruturas.h"

// Funções de gerenciamento de memória
void extrair_pagina_deslocamento(Simulador *sim, int endereco_virtual, int *pagina, int *deslocamento);
int verificar_pagina_presente(Simulador *sim, int pid, int pagina);
int traduzir_endereco(Simulador *sim, int pid, int endereco_virtual);
int carregar_pagina(Simulador *sim, int pid, int pagina);
int acessar_memoria(Simulador *sim, int pid, int endereco_virtual);

// Algoritmos de substituição de página
int substituir_pagina_fifo(Simulador *sim);
int substituir_pagina_lru(Simulador *sim);

#endif