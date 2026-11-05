#ifndef PROJETIL_H
#define PROJETIL_H

/* projetil.h - As babas que os cachorros cospem. Ficam numa LISTA
 * ENCADEADA: cada uma é criada com malloc no disparo e liberada com free
 * quando acerta um gato ou chega ao fim do campo. */

#include "raylib.h"
#include "enemy.h"
#include <stdbool.h>

#define VELOCIDADE_PROJETIL  200.0f   // pixels por segundo

typedef enum EstiloBaba {     // cada atirador cospe de um jeito
    BABA_GOTA,         // Beagle: uma gota de baba
    BABA_BOLHAS,       // Dálmata: bolhas de baba
    BABA_GELADA,       // Husky: baba congelada, deixa o gato lento
    BABA_JATO,         // Dachshund: um jato fino
    TOTAL_ESTILOS_BABA
} EstiloBaba;

typedef struct Projetil {
    Vector2    posicao;
    int        linha;          // só acerta gatos desta linha
    int        dano;
    EstiloBaba estilo;
    struct Projetil *proximo;  // próximo nó da lista (NULL = fim)
} Projetil;

bool projetil_adicionar(Projetil **lista, Vector2 posicao, int linha, int dano, EstiloBaba estilo);  // nova baba no início da lista (false se faltar memória)
int  projetil_atualizar_lista(Projetil **lista, Gato *gatos, float fimX, float dt);   // move, causa dano e remove; retorna o dano das que passaram de fimX
void projetil_desenhar_lista(const Projetil *lista);                              // desenha todas as babas
void projetil_liberar_lista(Projetil **lista);                                    // libera todos os nós e deixa a lista vazia

#endif
