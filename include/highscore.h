#ifndef HIGHSCORE_H
#define HIGHSCORE_H

/* highscore.h - O ranking (Top 10) das melhores vitórias, gravado em
 * assets/data/highscores.txt, uma linha por recorde: pontos fase tempo nome.
 * Os recordes ficam num VETOR ordenado do maior para o menor. */

#include "raylib.h"
#include <stdbool.h>

#define TOTAL_RECORDES     10
#define TAMANHO_NOME       12   // letras do nome do jogador
#define ARQUIVO_RECORDES   "assets/data/highscores.txt"

typedef struct Recorde {
    char  nome[TAMANHO_NOME + 1];
    int   fase;
    int   pontos;
    float tempo;            // segundos da partida
} Recorde;

typedef struct Ranking {
    Recorde lista[TOTAL_RECORDES];   // [0] é o 1º lugar
    int     total;                   // quantos recordes existem (até 10)
} Ranking;

void ranking_carregar(Ranking *ranking);                       // lê o arquivo (sem arquivo = ranking vazio)
bool ranking_gravar(const Ranking *ranking);                   // escreve o arquivo; false se não conseguir
int  ranking_posicao(const Ranking *ranking, int pontos);      // lugar que os pontos ocupariam (0 a 9), ou -1 se não entram
int  ranking_inserir(Ranking *ranking, const char *nome, int fase, int pontos, float tempo);  // põe o recorde no lugar certo; retorna o lugar ou -1
void ranking_desenhar(const Ranking *ranking, Rectangle area);  // tabela com lugar, nome, fase, pontos e tempo

#endif
