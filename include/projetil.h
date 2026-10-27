#ifndef PROJETIL_H
#define PROJETIL_H

/* projetil.h - Bolinhas de tênis atiradas pelos cachorros. Ficam numa LISTA
 * ENCADEADA: cada uma é criada com malloc no disparo e liberada com free
 * quando acerta um gato ou sai da tela. */

#include "raylib.h"
#include "enemy.h"
#include <stdbool.h>

#define VELOCIDADE_PROJETIL  200.0f   // pixels por segundo

typedef struct Projetil {
    Vector2 posicao;
    int     linha;             // só acerta gatos desta linha
    int     dano;
    bool    gelada;            // bolinha do Husky: deixa o gato lento
    struct Projetil *proximo;  // próximo nó da lista (NULL = fim)
} Projetil;

bool projetil_adicionar(Projetil **lista, Vector2 posicao, int linha, int dano, bool gelada);  // nova bolinha no início da lista (false se faltar memória)
void projetil_atualizar_lista(Projetil **lista, Gato *gatos, float dt);           // move, causa dano e remove as que acertaram ou saíram
void projetil_desenhar_lista(const Projetil *lista);                              // desenha todas as bolinhas
void projetil_liberar_lista(Projetil **lista);                                    // libera todos os nós e deixa a lista vazia

#endif
