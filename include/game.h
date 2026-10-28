#ifndef GAME_H
#define GAME_H

/* game.h - Estrutura principal do jogo e máquina de estados das telas.
 * Tudo é desenhado numa tela virtual de 640x360, depois ampliada para a
 * janela sem suavização (visual pixelado). */

#include "raylib.h"
#include "audio.h"
#include "deck.h"
#include "partida.h"
#include "save.h"

typedef enum EstadoJogo {
    ESTADO_MENU,        // tela inicial
    ESTADO_SAVES,       // escolha de um dos 3 saves
    ESTADO_FASES,       // as 5 fases do save escolhido
    ESTADO_INSTRUCOES,  // como jogar
    ESTADO_DECK,        // escolha do deck antes da fase
    ESTADO_JOGANDO,     // partida em andamento
    ESTADO_PAUSA,       // partida pausada
    ESTADO_SAIR         // fecha o jogo
} EstadoJogo;

typedef struct Jogo {
    EstadoJogo      estado;              // tela atual
    RenderTexture2D tela;                // tela virtual onde tudo é desenhado
    Vector2         mouse;               // mouse em coordenadas da tela virtual
    float           tempo;               // segundos desde a abertura (animações)
    Audio           audio;               // música de fundo
    Save            saves[TOTAL_SAVES];  // os 3 saves lidos do arquivo
    int             saveAtual;           // save em uso (-1 = nenhum)
    int             saveParaApagar;      // save esperando confirmação (-1 = nenhum)
    EscolhaDeck     deck;                // deck escolhido (lembrado entre as fases)
    Partida        *partida;             // partida em andamento (NULL fora dela)
} Jogo;

Jogo *jogo_criar(void);                 // aloca e prepara o jogo (depois de InitWindow)
void  jogo_atualizar(Jogo *jogo);       // lê a entrada e atualiza a tela atual
void  jogo_desenhar(const Jogo *jogo);  // desenha o frame e amplia para a janela
void  jogo_destruir(Jogo *jogo);        // libera tudo, inclusive o próprio jogo

#endif
