#ifndef PARTIDA_H
#define PARTIDA_H

/* partida.h - Uma partida em andamento (uma fase ou o tutorial). É alocada
 * quando a fase começa e liberada quando o jogador sai dela. */

#include "raylib.h"
#include "chefao.h"
#include "enemy.h"
#include "esteira.h"
#include "horda.h"
#include "map.h"
#include "petisco.h"
#include "projetil.h"
#include "tower.h"
#include <stdbool.h>

#define TAMANHO_DECK       5     // cachorros que o jogador escolhe antes da fase
#define MAX_CARTAS         (TAMANHO_DECK + 1)   // o Pote de Ração vem sempre junto
#define PETISCOS_INICIAIS  200
#define FASE_TUTORIAL      0
#define FASE_CHEFAO        5     // a fase do Rei dos Gatos, com a esteira
#define MAX_EFEITOS        8     // explosões/curas na tela ao mesmo tempo
#define USOS_LUVA          2     // vezes que a luva pode ser usada por partida
#define BONUS_POR_FASE     500   // pontos por vencer, vezes o número da fase
#define BONUS_REI          2000  // pontos a mais por derrotar o Rei dos Gatos
#define BONUS_TEMPO        1000  // bônus de tempo máximo (perde 1 ponto por segundo)

typedef enum ResultadoPartida {
    PARTIDA_EM_ANDAMENTO,
    PARTIDA_VITORIA,       // as 4 hordas, o Rei ou o gato do tutorial foram vencidos
    PARTIDA_DERROTA        // um gato invadiu a casa
} ResultadoPartida;

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

typedef struct Pontuacao {   // pontos de uma partida, separados para a tela de resultado
    int gatos;              // quantos gatos foram derrotados
    int pontosGatos;        // pontos ganhos com eles (cada tipo vale um tanto)
    int vitoria;            // bônus por vencer a fase (0 na derrota)
    int tempo;              // bônus por vencer depressa (0 na derrota)
    int total;
} Pontuacao;

typedef struct Partida {
    int      fase;               // 1 a 5, ou FASE_TUTORIAL
    Mapa     mapa;               // matriz 4x8 com os cachorros
    Petisco *petiscos;           // lista encadeada dos petiscos na tela
    Gato    *gatos;              // lista encadeada dos gatos em campo
    Projetil *projeteis;         // lista encadeada das bolinhas em voo
    int      saldo;              // petiscos disponíveis para gastar
    Carta    cartas[MAX_CARTAS]; // [0] é sempre o Pote de Ração
    int      totalCartas;        // 0 na fase do chefão (as cartas vêm pela esteira)
    Efeito   efeitos[MAX_EFEITOS];
    int      cartaSelecionada;   // índice em cartas[], ou -1
    bool     paSelecionada;
    bool     luvaSelecionada;
    int      usosLuva;           // usos da luva que ainda restam
    int      luvaLinha, luvaColuna;   // cachorro pego pela luva (luvaLinha -1 = nenhum)
    bool     acelerada;          // true = jogo em 2x
    float    tempoJogado;        // segundos de partida (tempo real, mesmo em 2x)
    float    timerPetiscoCeu;    // segundos até o próximo petisco do céu
    Hordas   hordas;             // as 4 hordas da fase (só nas fases 1 a 4)
    Chefao  *chefao;             // o Rei dos Gatos (só na fase do chefão; NULL nas outras)
    Esteira  esteira;            // cartas grátis da fase do chefão (vazia nas outras)
    int      gatosDerrotados[TOTAL_TIPOS_GATO];   // quantos gatos de cada tipo foram derrotados
    ResultadoPartida resultado;  // em andamento, vitória ou derrota
    int      passoTutorial;      // instrução atual do tutorial
    const char *aviso;           // mensagem temporária ("Petiscos insuficientes!")
    float    timerAviso;         // segundos restantes do aviso
} Partida;

Partida *partida_criar(int fase, const TipoCachorro deck[TAMANHO_DECK]);  // aloca a partida com o deck (NULL se faltar memória)
void partida_atualizar(Partida *partida, Vector2 mouse, float dt);  // entrada do jogador, petiscos e recargas
void partida_desenhar(const Partida *partida, Vector2 mouse);       // campo, barra superior e petiscos
void partida_destruir(Partida *partida);                            // libera a partida e tudo o que ela alocou
bool partida_eh_tutorial(const Partida *partida);                   // true se for o tutorial
void partida_adicionar_efeito(Partida *partida, Vector2 centro, float raio, Color cor);  // mostra uma explosão/cura
Pontuacao partida_pontuacao(const Partida *partida);                // pontos dos gatos, bônus e total

#endif
