#ifndef DECK_H
#define DECK_H

/* deck.h - Tela de escolha do deck: antes de cada fase o jogador escolhe 5
 * das 10 raças. O Pote de Ração vem sempre junto e não conta. */

#include "raylib.h"
#include "partida.h"
#include "tower.h"

#define TOTAL_RACAS  (TOTAL_TIPOS_CACHORRO - 1)   // todas as raças, menos o Pote de Ração

typedef struct EscolhaDeck {
    int          fase;                         // fase que vai começar
    TipoCachorro escolhidos[TAMANHO_DECK];     // raças escolhidas, na ordem
    int          total;                        // quantas já foram escolhidas
} EscolhaDeck;

typedef enum AcaoDeck {
    DECK_NADA,       // continua escolhendo
    DECK_COMECAR,    // deck completo: começar a fase
    DECK_VOLTAR      // voltar ao mapa de fases
} AcaoDeck;

void     deck_iniciar(EscolhaDeck *deck, int fase);                     // abre a tela da fase (mantém o último deck)
AcaoDeck deck_atualizar(EscolhaDeck *deck, Vector2 mouse);              // cliques nas cartas e nos botões
void     deck_desenhar(const EscolhaDeck *deck, Vector2 mouse, float tempo);  // desenha a tela de escolha

#endif
