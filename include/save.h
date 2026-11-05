#ifndef SAVE_H
#define SAVE_H

/* save.h - Os 3 espaços de save, gravados em assets/data/saves.txt, uma
 * linha por save: usado faseLiberada f1..f5 pontuacao tempo r1..r5 (r = o
 * recorde de pontos de cada fase). */

#include <stdbool.h>

#define TOTAL_SAVES    3
#define TOTAL_FASES    5
#define ARQUIVO_SAVES  "assets/data/saves.txt"

typedef struct Save {
    bool  usado;                       // false = espaço vazio
    int   faseLiberada;                // maior fase que já pode ser jogada
    bool  faseConcluida[TOTAL_FASES];  // fases já vencidas
    int   pontuacao;                   // soma dos recordes de todas as fases
    int   recorde[TOTAL_FASES];        // melhor pontuação em cada fase
    float tempoJogado;                 // segundos de partida acumulados
} Save;

void save_limpar(Save *save);                           // deixa o save vazio
void save_novo(Save *save);                             // jogo novo: só a fase 1 liberada
void save_carregar_todos(Save saves[TOTAL_SAVES]);      // lê o arquivo (sem arquivo = todos vazios)
bool save_gravar_todos(const Save saves[TOTAL_SAVES]);  // escreve o arquivo; false se não conseguir
int  save_fases_concluidas(const Save *save);           // quantas fases já foram vencidas
bool save_registrar_pontos(Save *save, int fase, int pontos);  // guarda os pontos se forem recorde da fase; true se foram

#endif
