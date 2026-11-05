#ifndef CHEFAO_H
#define CHEFAO_H

/* chefao.h - O Rei dos Gatos Zumbis, chefão da fase 5. Ele fica parado na
 * última coluna do campo, invoca gatos e, de tempos em tempos, rola uma
 * bola de pelo que derruba um cachorro. É criado com malloc só nessa fase,
 * e suas 4 hordas mudam conforme a vida dele cai. */

#include "raylib.h"
#include "enemy.h"
#include "map.h"
#include <stdbool.h>

#define VIDA_CHEFAO      30000
#define COLUNA_CHEFAO    (COLUNAS_MAPA - 1)   // coluna ocupada pelo Rei (nela não entram cachorros)
#define FRENTE_CHEFAO_X  (MAPA_X + COLUNA_CHEFAO * TAMANHO_CELULA + 16)   // as babas que chegam aqui o acertam

typedef struct Chefao {
    int   vida;
    int   horda;           // horda atual (0 a 3), conforme a vida que já perdeu
    float timerInvocar;    // segundos até invocar mais gatos
    float timerBola;       // segundos até rolar a próxima bola de pelo
    float timerAnuncio;    // segundos restantes do anúncio da horda
    float timerDano;       // segundos com a cor de dano
    float animacao;        // segundos desde o início (respiração)
    int   bolaLinha;       // linha da bola de pelo rolando (-1 = nenhuma)
    float bolaX;           // posição horizontal da bola
} Chefao;

Chefao *chefao_criar(void);                                                 // aloca o chefão com a vida cheia (NULL se faltar memória)
void  chefao_destruir(Chefao *chefao);                                      // libera o chefão (aceita NULL)
void  chefao_atualizar(Chefao *chefao, Gato **gatos, Mapa *mapa, float dt); // troca de horda, invoca gatos e rola a bola
void  chefao_ferir(Chefao *chefao, int dano);                               // tira vida e muda a cor por um instante
bool  chefao_derrotado(const Chefao *chefao);                               // true se a vida acabou
float chefao_fracao_vida(const Chefao *chefao);                             // vida restante, de 0 a 1
const char *chefao_anuncio(const Chefao *chefao);                           // texto do anúncio da horda (NULL se nenhum)
void  chefao_desenhar(const Chefao *chefao);                                // o território, o Rei com a coroa e a bola
void  chefao_desenhar_vida(const Chefao *chefao, Rectangle area);           // nome e barra de vida na barra superior

#endif
