#ifndef TOWER_H
#define TOWER_H

/* tower.h - Os defensores: as raças de cachorro e o Pote de Ração. Cada
 * um é desenhado com o seu sprite; sem o PNG, vira uma forma geométrica. */

#include "raylib.h"
#include "sprite.h"

#define RAIO_CACHORRO  22   // tamanho do cachorro desenhado no campo

typedef enum TipoCachorro {
    TIPO_POTE_RACAO,   // gerador de petiscos: não é cachorro e não conta como carta
    TIPO_BEAGLE,
    TIPO_BULLDOG,
    TIPO_DALMATA,
    TIPO_HUSKY,
    TIPO_PASTOR_ALEMAO,
    TIPO_WESTIE,       // West Highland White Terrier
    TIPO_SAO_BERNARDO,
    TIPO_DACHSHUND,
    TIPO_ROTTWEILER,
    TIPO_SHIBA,
    TOTAL_TIPOS_CACHORRO
} TipoCachorro;

typedef struct InfoCachorro {   // dados fixos de cada raça (provisórios)
    const char *nome;
    const char *papel;     // função na defesa
    int   custo;           // preço em petiscos
    float recarga;         // segundos até poder comprar outro
    int   vidaMaxima;
    int   dano;            // dano de cada ataque (no São Bernardo, a cura)
    float cadencia;        // segundos entre duas ações
    float inicio;          // segundos até a 1ª ação (o pote produzir, a mina armar...)
    IdSprite sprite;       // animação parada (o pote: o gerador de petiscos)
    int   lados;           // forma reserva, sem o PNG: 0 = círculo, 3 = triângulo, 4 = quadrado...
    float giro;            // rotação da forma, em graus
    Color cor;
} InfoCachorro;

typedef struct Cachorro {   // um cachorro (ou pote) no campo, criado com malloc
    TipoCachorro tipo;
    int   linha, coluna;   // posição na matriz do mapa
    int   vida;
    float timer;           // segundos até a próxima ação (petisco do pote, ataque...)
    float animacao;        // segundos da animação parada já tocados
} Cachorro;

const InfoCachorro *cachorro_info(TipoCachorro tipo);                    // dados fixos de um tipo
Cachorro *cachorro_criar(TipoCachorro tipo, int linha, int coluna);      // aloca um cachorro (NULL se faltar memória)
void cachorro_destruir(Cachorro *cachorro);                              // libera um cachorro
int  cachorro_quadro(const Cachorro *cachorro);                          // quadro da animação no campo
void cachorro_desenhar(TipoCachorro tipo, Vector2 centro, float raio, int quadro, Color tinta);  // desenha o tipo (sprite ou forma)
void cachorro_desenhar_retrato(TipoCachorro tipo, Vector2 centro);       // a cabeça do tipo em tamanho real, para as cartas

#endif
