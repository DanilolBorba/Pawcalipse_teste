#ifndef MAP_H
#define MAP_H

/* map.h - O campo de batalha: uma matriz 4x8 de ponteiros para cachorros,
 * com a casa à esquerda e a rua (de onde vêm os gatos) à direita. */

#include "raylib.h"
#include "tower.h"
#include <stdbool.h>

#define LINHAS_MAPA     4
#define COLUNAS_MAPA    8
#define TAMANHO_CELULA  64
#define MAPA_X          64   // canto superior esquerdo do campo
#define MAPA_Y          96

typedef struct Mapa {
    Cachorro *grade[LINHAS_MAPA][COLUNAS_MAPA];   // NULL = célula livre
} Mapa;

void      mapa_iniciar(Mapa *mapa);                                      // deixa todas as células vazias
void      mapa_limpar(Mapa *mapa);                                       // libera todos os cachorros
bool      mapa_celula_no_ponto(Vector2 ponto, int *linha, int *coluna);  // ponto da tela -> célula (false se fora)
Rectangle mapa_retangulo_celula(int linha, int coluna);                  // área da célula na tela
Vector2   mapa_centro_celula(int linha, int coluna);                     // centro da célula na tela
bool      mapa_colocar(Mapa *mapa, Cachorro *cachorro);                  // guarda o cachorro na célula dele (false se ocupada)
void      mapa_remover(Mapa *mapa, int linha, int coluna);               // libera o cachorro e esvazia a célula
bool      mapa_mover(Mapa *mapa, Cachorro *cachorro, int linha, int coluna);  // muda o cachorro de célula (false se o destino estiver ocupado)
void      mapa_desenhar(const Mapa *mapa);                               // gramado, casa, rua e cachorros

#endif
