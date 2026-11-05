/* tower.c - Tabela das raças e o desenho dos cachorros e do pote (sprites,
 * com formas geométricas como reserva). */

#include "tower.h"

#include <stdlib.h>

static const InfoCachorro infos[TOTAL_TIPOS_CACHORRO] = {
    /*                      nome             papel             custo recarga  vida  dano cadência início  sprite                lados  giro    cor */
    [TIPO_POTE_RACAO]    = { "Pote de Ração", "Gera petiscos",     50,  7.5f,  300,    0, 15.0f,  6.0f,  SPRITE_GERADOR,       0,   0.0f, { 196,  58,  50, 255 } },
    [TIPO_BEAGLE]        = { "Beagle",        "Atirador",         100,  7.5f,  300,   20,  1.5f,  0.0f,  SPRITE_BEAGLE,        0,   0.0f, { 196, 140,  80, 255 } },
    [TIPO_BULLDOG]       = { "Bulldog",       "Muralha",           50, 30.0f, 4000,    0,  0.0f,  0.0f,  SPRITE_BULLDOG,       4,  45.0f, { 214, 180, 140, 255 } },
    [TIPO_DALMATA]       = { "Dálmata",       "Atirador duplo",   200,  7.5f,  300,   20,  1.5f,  0.0f,  SPRITE_DALMATA,       5, -90.0f, { 240, 240, 236, 255 } },
    [TIPO_HUSKY]         = { "Husky",         "Latido gelado",    175,  7.5f,  300,   20,  1.5f,  0.0f,  SPRITE_HUSKY,         3, -90.0f, { 150, 196, 236, 255 } },
    [TIPO_PASTOR_ALEMAO] = { "Pastor Alemão", "Mordida forte",    150,  7.5f,  300,   40,  1.0f,  0.0f,  SPRITE_PASTOR_ALEMAO, 6,   0.0f, { 190, 130,  64, 255 } },
    [TIPO_WESTIE]        = { "Westie",        "Toca-armadilha",    25, 30.0f,  300,  600,  0.0f, 10.0f,  SPRITE_WESTIE,        4,   0.0f, { 245, 245, 238, 255 } },
    [TIPO_SAO_BERNARDO]  = { "São Bernardo",  "Cura vizinhos",    125, 15.0f,  300,   50,  5.0f,  0.0f,  SPRITE_SAO_BERNARDO,  8,  22.5f, { 150,  80,  40, 255 } },
    [TIPO_DACHSHUND]     = { "Dachshund",     "Ataca 3 linhas",   300,  7.5f,  300,   20,  1.5f,  0.0f,  SPRITE_DACHSHUND,     0,   0.0f, {  34,  30,  32, 255 } },
    [TIPO_ROTTWEILER]    = { "Rottweiler",    "Explosão 3x3",     150, 35.0f,  300, 1800,  0.0f,  1.0f,  SPRITE_ROTTWEILER,    3,  90.0f, {  36,  32,  34, 255 } },
    [TIPO_SHIBA]         = { "Shiba Inu",     "Grito assustador", 125, 15.0f,  300,    0, 10.0f,  0.0f,  SPRITE_SHIBA,         7, -90.0f, { 214, 120,  52, 255 } },
};

/* cachorro_info: retorna um ponteiro para os dados fixos do tipo na tabela. */
const InfoCachorro *cachorro_info(TipoCachorro tipo)
{
    return &infos[tipo];
}

/* cachorro_criar: aloca um cachorro com malloc, com a vida cheia do tipo e a
 * posição na matriz. O timer começa com o tempo até a 1ª ação do tipo, e a
 * animação, num quadro sorteado (para os cachorros não abanarem o rabo
 * todos juntos).
 * Retorna o ponteiro, ou NULL se faltar memória. */
Cachorro *cachorro_criar(TipoCachorro tipo, int linha, int coluna)
{
    Cachorro *cachorro = malloc(sizeof(Cachorro));
    if (cachorro == NULL) return NULL;

    cachorro->tipo = tipo;
    cachorro->linha = linha;
    cachorro->coluna = coluna;
    cachorro->vida = infos[tipo].vidaMaxima;
    cachorro->timer = infos[tipo].inicio;
    cachorro->animacao = (rand() % 8) * 0.12f;
    return cachorro;
}

/* cachorro_destruir: libera a memória do cachorro (aceita NULL). */
void cachorro_destruir(Cachorro *cachorro)
{
    free(cachorro);
}

/* cachorro_quadro: retorna o quadro da animação do cachorro no campo. Os
 * cachorros tocam a animação parada em loop. O pote fica parado no 1º quadro
 * e, perto de produzir, toca os outros de modo que o quadro da estrela
 * coincida com o nascimento do petisco (quando o timer chega a zero e volta
 * para a cadência). */
int cachorro_quadro(const Cachorro *cachorro)
{
    if (cachorro->tipo != TIPO_POTE_RACAO) return sprite_quadro_no_tempo(infos[cachorro->tipo].sprite, cachorro->animacao);

    float estrela = sprite_duracao_ate(SPRITE_GERADOR, QUADRO_ESTRELA_GERADOR);
    float antes = estrela - sprite_duracao_ate(SPRITE_GERADOR, 1);
    float depois = sprite_duracao_total(SPRITE_GERADOR) - estrela;
    float desdeOPetisco = infos[TIPO_POTE_RACAO].cadencia - cachorro->timer;

    if (cachorro->timer <= antes) return sprite_quadro_no_tempo(SPRITE_GERADOR, estrela - cachorro->timer);
    if (desdeOPetisco < depois)   return sprite_quadro_no_tempo(SPRITE_GERADOR, estrela + desdeOPetisco);
    return 0;
}

/* cachorro_desenhar: desenha o tipo centrado em 'centro', com tamanho 'raio'
 * (RAIO_CACHORRO = tamanho no campo, escala 1 do sprite). Com o PNG, usa o
 * quadro pedido do sprite, com os pés perto da base da célula; sem ele, o
 * pote vira uma tigela e as raças, polígonos com contorno escuro (círculo =
 * 32 lados). 'tinta' multiplica as cores (WHITE = normal; transparente =
 * prévia). */
void cachorro_desenhar(TipoCachorro tipo, Vector2 centro, float raio, int quadro, Color tinta)
{
    const InfoCachorro *info = &infos[tipo];
    Color cor = ColorTint(info->cor, tinta);

    if (sprite_existe(info->sprite)) {
        sprite_desenhar(info->sprite, quadro, (Vector2){ centro.x, centro.y + raio * 1.35f }, raio / RAIO_CACHORRO, tinta);
        return;
    }
    if (tipo == TIPO_POTE_RACAO) {
        DrawCircleV((Vector2){ centro.x, centro.y - 2 }, raio * 0.75f, ColorTint((Color){ 150, 96, 50, 255 }, tinta));
        DrawCircleSector(centro, raio, 0, 180, 16, cor);
        return;
    }

    int lados = (info->lados == 0) ? 32 : info->lados;
    DrawPoly(centro, lados, raio, info->giro, cor);
    DrawPolyLinesEx(centro, lados, raio, info->giro, 2, ColorTint(ColorBrightness(info->cor, -0.5f), tinta));
}
