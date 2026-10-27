#ifndef SAVE_H
#define SAVE_H

/* save.h - Os 3 espaços de save, gravados em assets/data/saves.txt, uma
 * linha por save: usado faseLiberada f1 f2 f3 f4 f5 pontuacao tempo */

#include <stdbool.h>

#define TOTAL_SAVES    3
#define TOTAL_FASES    5
#define ARQUIVO_SAVES  "assets/data/saves.txt"

typedef struct Save {
    bool  usado;                       // false = espaço vazio
    int   faseLiberada;                // maior fase que já pode ser jogada
    bool  faseConcluida[TOTAL_FASES];  // fases já vencidas
    int   pontuacao;                   // pontos somados de todas as fases
    float tempoJogado;                 // segundos de partida acumulados
} Save;

void save_limpar(Save *save);                           // deixa o save vazio
void save_novo(Save *save);                             // jogo novo: só a fase 1 liberada
void save_carregar_todos(Save saves[TOTAL_SAVES]);      // lê o arquivo (sem arquivo = todos vazios)
bool save_gravar_todos(const Save saves[TOTAL_SAVES]);  // escreve o arquivo; false se não conseguir
int  save_fases_concluidas(const Save *save);           // quantas fases já foram vencidas

#endif
