#ifndef SPRITE_H
#define SPRITE_H

/* sprite.h - Spritesheets animados: um PNG com os quadros de 64x64 lado a
 * lado. Se um arquivo não existir, quem desenha usa as formas geométricas. */

#include "raylib.h"
#include <stdbool.h>

#define TAMANHO_QUADRO          64   // largura e altura de cada quadro
#define MAX_QUADROS             12
#define QUADRO_ESTRELA_GERADOR  8    // quadro do gerador em que o petisco nasce (a estrela)

typedef enum IdSprite {
    SPRITE_GATO_PADRAO,
    SPRITE_GATO_CAIXA,
    SPRITE_GATO_LATA,
    SPRITE_GERADOR,
    TOTAL_SPRITES
} IdSprite;

void  sprite_carregar_todos(void);                     // carrega os PNGs (depois de InitWindow)
void  sprite_descarregar_todos(void);                  // libera as texturas
bool  sprite_existe(IdSprite id);                      // false se o PNG não foi carregado
float sprite_duracao_ate(IdSprite id, int quadro);     // segundos do início da animação até o começo do quadro
float sprite_duracao_total(IdSprite id);               // segundos de um ciclo inteiro
int   sprite_quadro_no_tempo(IdSprite id, float tempo);                                      // quadro a mostrar 'tempo' segundos após o início (em loop)
void  sprite_desenhar(IdSprite id, int quadro, Vector2 posicao, float escala, Color tinta);  // desenha o quadro com a origem do sprite em 'posicao'

#endif
