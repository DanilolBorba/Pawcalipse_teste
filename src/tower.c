/* tower.c - Tabela das raças e o desenho (sprite do pote e formas geométricas
 * provisórias para os cachorros). */

#include "tower.h"
#include "sprite.h"

#include <stdlib.h>

static const InfoCachorro infos[TOTAL_TIPOS_CACHORRO] = {
    /*                      nome             papel             custo recarga  vida  dano cadência início lados  giro    cor */
    [TIPO_POTE_RACAO]    = { "Pote de Ração", "Gera petiscos",     50,  7.5f,  300,    0, 15.0f,  6.0f,  0,   0.0f, { 196,  58,  50, 255 } },
    [TIPO_BEAGLE]        = { "Beagle",        "Atirador",         100,  7.5f,  300,   20,  1.5f,  0.0f,  0,   0.0f, { 196, 140,  80, 255 } },
    [TIPO_BULLDOG]       = { "Bulldog",       "Muralha",           50, 30.0f, 4000,    0,  0.0f,  0.0f,  4,  45.0f, { 214, 180, 140, 255 } },
    [TIPO_DALMATA]       = { "Dálmata",       "Atirador duplo",   200,  7.5f,  300,   20,  1.5f,  0.0f,  5, -90.0f, { 240, 240, 236, 255 } },
    [TIPO_HUSKY]         = { "Husky",         "Latido gelado",    175,  7.5f,  300,   20,  1.5f,  0.0f,  3, -90.0f, { 150, 196, 236, 255 } },
    [TIPO_PASTOR_ALEMAO] = { "Pastor Alemão", "Mordida forte",    150,  7.5f,  300,   40,  1.0f,  0.0f,  6,   0.0f, { 190, 130,  64, 255 } },
    [TIPO_CHIHUAHUA]     = { "Chihuahua",     "Mina explosiva",    25, 30.0f,  300,  600,  0.0f, 10.0f,  4,   0.0f, { 240, 204,  90, 255 } },
    [TIPO_SAO_BERNARDO]  = { "São Bernardo",  "Cura vizinhos",    125, 15.0f,  300,   50,  5.0f,  0.0f,  8,  22.5f, { 150,  80,  40, 255 } },
    [TIPO_DACHSHUND]     = { "Dachshund",     "Ataca 3 linhas",   300,  7.5f,  300,   20,  1.5f,  0.0f,  0,   0.0f, {  34,  30,  32, 255 } },
    [TIPO_ROTTWEILER]    = { "Rottweiler",    "Explosão 3x3",     150, 35.0f,  300, 1800,  0.0f,  1.0f,  3,  90.0f, {  36,  32,  34, 255 } },
};

/* cachorro_info: retorna um ponteiro para os dados fixos do tipo na tabela. */
const InfoCachorro *cachorro_info(TipoCachorro tipo)
{
    return &infos[tipo];
}

/* cachorro_criar: aloca um cachorro com malloc, com a vida cheia do tipo e a
 * posição na matriz. O timer começa com o tempo até a 1ª ação do tipo.
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
    return cachorro;
}

/* cachorro_destruir: libera a memória do cachorro (aceita NULL). */
void cachorro_destruir(Cachorro *cachorro)
{
    free(cachorro);
}

/* cachorro_quadro: retorna o quadro da animação do cachorro no campo. Só o
 * pote anima: fica parado no 1º quadro e, perto de produzir, toca os outros
 * de modo que o quadro da estrela coincida com o nascimento do petisco
 * (quando o timer chega a zero e volta para a cadência). */
int cachorro_quadro(const Cachorro *cachorro)
{
    if (cachorro->tipo != TIPO_POTE_RACAO) return 0;

    float estrela = sprite_duracao_ate(SPRITE_GERADOR, QUADRO_ESTRELA_GERADOR);
    float antes = estrela - sprite_duracao_ate(SPRITE_GERADOR, 1);
    float depois = sprite_duracao_total(SPRITE_GERADOR) - estrela;
    float desdeOPetisco = infos[TIPO_POTE_RACAO].cadencia - cachorro->timer;

    if (cachorro->timer <= antes) return sprite_quadro_no_tempo(SPRITE_GERADOR, estrela - cachorro->timer);
    if (desdeOPetisco < depois)   return sprite_quadro_no_tempo(SPRITE_GERADOR, estrela + desdeOPetisco);
    return 0;
}

/* desenhar_dachshund: o Dachshund (provisório até o sprite), virado para a
 * direita: corpo comprido e cabeça pretos, com patas, focinho e sobrancelha
 * marrons. */
static void desenhar_dachshund(Vector2 c, float r, Color tinta)
{
    Color preto = ColorTint(infos[TIPO_DACHSHUND].cor, tinta);
    Color marrom = ColorTint((Color){ 160, 92, 44, 255 }, tinta);

    DrawRectangle(c.x - r * 0.8f, c.y + r * 0.2f, r * 0.25f, r * 0.45f, marrom);
    DrawRectangle(c.x + r * 0.45f, c.y + r * 0.2f, r * 0.25f, r * 0.45f, marrom);
    DrawRectangleRounded((Rectangle){ c.x - r, c.y - r * 0.35f, r * 1.7f, r * 0.7f }, 1.0f, 8, preto);
    DrawCircleV((Vector2){ c.x + r * 0.75f, c.y - r * 0.45f }, r * 0.4f, preto);
    DrawCircleV((Vector2){ c.x + r * 1.05f, c.y - r * 0.35f }, r * 0.2f, marrom);
    DrawCircleV((Vector2){ c.x + r * 0.8f, c.y - r * 0.65f }, r * 0.1f + 1, marrom);
}

/* cachorro_desenhar: desenha o tipo centrado em 'centro', com tamanho 'raio'
 * (RAIO_CACHORRO = tamanho no campo). O Pote de Ração usa o quadro pedido do
 * sprite do gerador (ou, sem o PNG, uma tigela); o Dachshund tem desenho
 * próprio; as outras raças são polígonos com contorno escuro (círculo = 32
 * lados). 'tinta' multiplica as cores (WHITE = normal; transparente = prévia). */
void cachorro_desenhar(TipoCachorro tipo, Vector2 centro, float raio, int quadro, Color tinta)
{
    const InfoCachorro *info = &infos[tipo];
    Color cor = ColorTint(info->cor, tinta);

    if (tipo == TIPO_POTE_RACAO && sprite_existe(SPRITE_GERADOR)) {
        sprite_desenhar(SPRITE_GERADOR, quadro, (Vector2){ centro.x, centro.y + raio * 1.35f }, raio / RAIO_CACHORRO, tinta);
        return;
    }
    if (tipo == TIPO_POTE_RACAO) {
        DrawCircleV((Vector2){ centro.x, centro.y - 2 }, raio * 0.75f, ColorTint((Color){ 150, 96, 50, 255 }, tinta));
        DrawCircleSector(centro, raio, 0, 180, 16, cor);
        return;
    }
    if (tipo == TIPO_DACHSHUND) {
        desenhar_dachshund(centro, raio, tinta);
        return;
    }

    int lados = (info->lados == 0) ? 32 : info->lados;
    DrawPoly(centro, lados, raio, info->giro, cor);
    DrawPolyLinesEx(centro, lados, raio, info->giro, 2, ColorTint(ColorBrightness(info->cor, -0.5f), tinta));
}
