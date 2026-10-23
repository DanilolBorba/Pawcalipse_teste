#ifndef GAME_H
#define GAME_H

/* game.h - Estrutura principal do jogo e máquina de estados das telas.
 * Tudo é desenhado numa tela virtual de 640x360, depois ampliada para a
 * janela sem suavização (visual pixelado). */

#include "raylib.h"

typedef enum EstadoJogo {
    ESTADO_MENU,        // tela inicial
    ESTADO_INSTRUCOES,  // como jogar
    ESTADO_JOGANDO,     // partida em andamento
    ESTADO_PAUSA,       // partida pausada
    ESTADO_SAIR         // fecha o jogo
} EstadoJogo;

typedef struct Jogo {
    EstadoJogo      estado;  // tela atual
    RenderTexture2D tela;    // tela virtual onde tudo é desenhado
    Vector2         mouse;   // mouse em coordenadas da tela virtual
    float           tempo;   // segundos desde a abertura (animações)
} Jogo;

Jogo *jogo_criar(void);                 // aloca e prepara o jogo (depois de InitWindow)
void  jogo_atualizar(Jogo *jogo);       // lê a entrada e atualiza a tela atual
void  jogo_desenhar(const Jogo *jogo);  // desenha o frame e amplia para a janela
void  jogo_destruir(Jogo *jogo);        // libera tudo, inclusive o próprio jogo

#endif
