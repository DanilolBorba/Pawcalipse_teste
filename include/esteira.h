#ifndef ESTEIRA_H
#define ESTEIRA_H

/* esteira.h - A esteira da fase do chefão: em vez de comprar cartas com
 * petiscos, o jogador recebe cachorros de graça, que sobem por uma esteira
 * à esquerda do campo. As cartas ficam num VETOR, na ordem de chegada. */

#include "raylib.h"
#include "tower.h"

#define MAX_ESTEIRA  7   // cartas que cabem na esteira (cheia, não chegam outras)

typedef struct CartaEsteira {
    TipoCachorro tipo;
    float        y;        // topo da carta na tela (ela sobe até a sua vaga)
} CartaEsteira;

typedef struct Esteira {
    CartaEsteira cartas[MAX_ESTEIRA];   // [0] é a de cima
    int   total;
    int   selecionada;     // índice em cartas[], ou -1
    float timer;           // segundos até a próxima carta chegar
    float movimento;       // quanto a esteira já andou (animação das listras)
} Esteira;

void esteira_iniciar(Esteira *esteira);                               // esteira vazia, com a 1ª carta já chegando
void esteira_atualizar(Esteira *esteira, float dt);                   // traz cartas novas e faz as cartas subirem
int  esteira_carta_no_ponto(const Esteira *esteira, Vector2 ponto);   // índice da carta sob o ponto, ou -1
void esteira_remover(Esteira *esteira, int i);                        // tira a carta i (as de baixo sobem)
void esteira_desenhar(const Esteira *esteira, Vector2 mouse);         // a faixa, as cartas e a dica da carta sob o mouse

#endif
