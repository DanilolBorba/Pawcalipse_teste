#ifndef PARTIDA_H
#define PARTIDA_H

/* partida.h - Uma partida em andamento (uma fase ou o tutorial). É alocada
 * quando a fase começa e liberada quando o jogador sai dela. */

#include "raylib.h"
#include "enemy.h"
#include "map.h"
#include "petisco.h"
#include "projetil.h"
#include "tower.h"
#include <stdbool.h>

#define MAX_CARTAS         6     // Pote de Ração + 5 cachorros do deck
#define PETISCOS_INICIAIS  200
#define FASE_TUTORIAL      0
#define MAX_EFEITOS        8     // explosões/curas na tela ao mesmo tempo

typedef struct Carta {
    TipoCachorro tipo;
    float        recarga;   // segundos até poder comprar de novo
} Carta;

typedef struct Efeito {     // círculo que cresce e some (explosão, cura)
    Vector2 centro;
    float   raio;
    float   tempo;          // segundos restantes (0 = vaga livre no vetor)
    Color   cor;
} Efeito;

typedef struct Partida {
    int      fase;               // 1 a 5, ou FASE_TUTORIAL
    Mapa     mapa;               // matriz 4x8 com os cachorros
    Petisco *petiscos;           // lista encadeada dos petiscos na tela
    Gato    *gatos;              // lista encadeada dos gatos em campo
    Projetil *projeteis;         // lista encadeada das bolinhas em voo
    int      saldo;              // petiscos disponíveis para gastar
    Carta    cartas[MAX_CARTAS]; // [0] é sempre o Pote de Ração
    int      totalCartas;
    int      deckTeste;          // deck provisório em uso (TAB troca; até a escolha de deck)
    Efeito   efeitos[MAX_EFEITOS];
    int      cartaSelecionada;   // índice em cartas[], ou -1
    bool     paSelecionada;
    float    tempoJogado;        // segundos de partida
    float    timerPetiscoCeu;    // segundos até o próximo petisco do céu
    float    timerGato;          // segundos até o próximo gato (provisório, até as hordas)
    bool     derrota;            // true quando um gato invade a casa
    int      passoTutorial;      // instrução atual do tutorial
    const char *aviso;           // mensagem temporária ("Petiscos insuficientes!")
    float    timerAviso;         // segundos restantes do aviso
} Partida;

Partida *partida_criar(int fase);                                   // aloca a partida (NULL se faltar memória)
void partida_atualizar(Partida *partida, Vector2 mouse, float dt);  // entrada do jogador, petiscos e recargas
void partida_desenhar(const Partida *partida, Vector2 mouse);       // campo, barra superior e petiscos
void partida_destruir(Partida *partida);                            // libera a partida e tudo o que ela alocou
bool partida_eh_tutorial(const Partida *partida);                   // true se for o tutorial
void partida_adicionar_efeito(Partida *partida, Vector2 centro, float raio, Color cor);  // mostra uma explosão/cura

#endif
