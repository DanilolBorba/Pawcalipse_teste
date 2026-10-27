#ifndef TOWER_H
#define TOWER_H

/* tower.h - Os defensores: as raças de cachorro e o Pote de Ração.
 * Até os sprites ficarem prontos, cada raça é uma forma geométrica. */

#include "raylib.h"

#define POTE_PRIMEIRA_PRODUCAO   6.0f   // segundos até o pote soltar o 1º petisco
#define POTE_INTERVALO_PRODUCAO 15.0f   // segundos entre os petiscos seguintes

typedef enum TipoCachorro {
    TIPO_POTE_RACAO,   // gerador de petiscos: não é cachorro e não conta como carta
    TIPO_BEAGLE,
    TIPO_BULLDOG,
    TIPO_DALMATA,
    TIPO_HUSKY,
    TIPO_PASTOR_ALEMAO,
    TIPO_CHIHUAHUA,
    TIPO_SAO_BERNARDO,
    TIPO_BORDER_COLLIE,
    TIPO_ROTTWEILER,
    TOTAL_TIPOS_CACHORRO
} TipoCachorro;

typedef struct InfoCachorro {   // dados fixos de cada raça (provisórios)
    const char *nome;
    const char *papel;     // função na defesa
    int   custo;           // preço em petiscos
    float recarga;         // segundos até poder comprar outro
    int   vidaMaxima;
    int   dano;            // dano de cada ataque (0 = não ataca)
    float cadencia;        // segundos entre dois ataques
    int   lados;          // forma: 0 = círculo, 3 = triângulo, 4 = quadrado...
    float giro;            // rotação da forma, em graus
    Color cor;
} InfoCachorro;

typedef struct Cachorro {   // um cachorro (ou pote) no campo, criado com malloc
    TipoCachorro tipo;
    int   linha, coluna;   // posição na matriz do mapa
    int   vida;
    float timer;           // segundos até a próxima ação (petisco do pote, ataque...)
} Cachorro;

const InfoCachorro *cachorro_info(TipoCachorro tipo);                    // dados fixos de um tipo
Cachorro *cachorro_criar(TipoCachorro tipo, int linha, int coluna);      // aloca um cachorro (NULL se faltar memória)
void cachorro_destruir(Cachorro *cachorro);                              // libera um cachorro
void cachorro_desenhar(TipoCachorro tipo, Vector2 centro, float raio, Color tinta);  // desenha a forma do tipo

#endif
