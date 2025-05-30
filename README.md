# Simulador de Paginação de Memória

Este projeto implementa um simulador de paginação de memória para a disciplina de Sistemas Operacionais, demonstrando o funcionamento de diferentes algoritmos de substituição de páginas.

## Integrantes do Grupo

- Guilherme Teodoro de Oliveira RA: 10425362
- Luís Henrique Ribeiro Fernandes RA: 10420079  
- Vinícius Brait Lorimier RA: 10420046

## Descrição do Projeto

O simulador implementa um sistema de paginação de memória com as seguintes características:

- Tradução de endereços virtuais para endereços físicos
- Gerenciamento de tabelas de páginas para múltiplos processos
- Implementação de algoritmos de substituição de páginas:
  - **FIFO** (First-In-First-Out)
  - **LRU** (Least Recently Used)

O simulador registra estatísticas importantes como o número de acessos à memória, page faults e a taxa de page faults, permitindo comparar o desempenho dos diferentes algoritmos de substituição.

## Configurações do Simulador

- **Tamanho da página**: 4096 bytes (4KB)
- **Tamanho da memória física**: 16384 bytes (16KB) - 4 frames
- **Número de processos**: 3 processos simultâneos
- **Tamanho de cada processo**: 16384 bytes (4 páginas cada)

## Estrutura do Projeto

```
ProjetoSO_2Bimestre/
├── doc/
│     └── doc/Projeto2_SimuladorDePaginaçãoDeMemória.pdf # Relatório principal do projeto
├── estruturas.h         # Definições de estruturas e constantes
├── memoria.h            # Cabeçalhos das funções de gerenciamento de memória
├── memoria.c            # Implementação do gerenciamento de memória e algoritmos
├── simulador.h          # Cabeçalhos das funções do simulador e interface
├── simulador.c          # Implementação do simulador e interface do usuário
├── main.c               # Arquivo principal
└── README.md            # Documentação auxiliar do projeto
```

## Relatório

O relatório completo do projeto está disponível [aqui](doc/Projeto2_SimuladorDePaginaçãoDeMemória.pdf).

## Funcionalidades Implementadas

### Algoritmos de Substituição
- **FIFO**: Remove a página que está há mais tempo na memória
- **LRU**: Remove a página que foi acessada há mais tempo

### Recursos do Simulador
- Detecção e tratamento de page faults
- Estatísticas detalhadas de desempenho
- Interface interativa para seleção de algoritmos

## Instruções de Compilação e Execução

### Requisitos
- Compilador GCC
- Sistema operacional Linux/Unix ou Windows com MinGW

### Compilação

**Comando de compilação:**
```bash
gcc -o simulador main.c simulador.c memoria.c
```

### Execução
```bash
./simulador
```

## Como Usar o Simulador

1. Execute o programa
2. Escolha o algoritmo de substituição:
   - **1** - FIFO
   - **2** - LRU  
   - **3** - Sair
3. O simulador executará uma sequência pré-definida de 20 acessos à memória
4. Observe os resultados em tempo real:
   - Tradução de endereços
   - Ocorrência de page faults
   - Estado da memória física
   - Estatísticas finais

## Exemplo de Saída

```
=== INICIANDO SIMULAÇÃO COM FIFO ===

--- Acesso 1: P1, Endereço 1111 ---
Tempo t=0: Endereço Virtual (P1): 1111 -> Página: 0 -> Frame: 0 -> Endereço Físico: 1111

Estado da Memória Física:
--------
| P1-0 |
--------
| ---- |
--------
| ---- |
--------
| ---- |
--------

======== ESTATÍSTICAS DA SIMULAÇÃO ========
Total de acessos à memória: 20
Total de page faults: 12
Taxa de page faults: 60.00%
```