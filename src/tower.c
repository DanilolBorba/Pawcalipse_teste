/* tower.c - Tabela das raças e o desenho provisório (formas geométricas). */

#include "tower.h"

#include <stdlib.h>

static const InfoCachorro infos[TOTAL_TIPOS_CACHORRO] = {
    /*                      nome             papel             custo recarga  vida lados  giro    cor */
    [TIPO_POTE_RACAO]    = { "Pote de Ração", "Gera petiscos",     50,  7.5f,  300,  0,   0.0f, { 196,  58,  50, 255 } },
    [TIPO_BEAGLE]        = { "Beagle",        "Atirador",         100,  7.5f,  300,  0,   0.0f, { 196, 140,  80, 255 } },
    [TIPO_BULLDOG]       = { "Bulldog",       "Muralha",           50, 30.0f, 4000,  4,  45.0f, { 214, 180, 140, 255 } },
    [TIPO_DALMATA]       = { "Dálmata",       "Atirador duplo",   200,  7.5f,  300,  5, -90.0f, { 240, 240, 236, 255 } },
    [TIPO_HUSKY]         = { "Husky",         "Latido gelado",    175,  7.5f,  300,  3, -90.0f, { 150, 196, 236, 255 } },
    [TIPO_PASTOR_ALEMAO] = { "Pastor Alemão", "Mordida forte",    150,  7.5f,  300,  6,   0.0f, { 190, 130,  64, 255 } },
    [TIPO_CHIHUAHUA]     = { "Chihuahua",     "Mina explosiva",    25, 30.0f,  300,  4,   0.0f, { 240, 204,  90, 255 } },
    [TIPO_SAO_BERNARDO]  = { "São Bernardo",  "Cura vizinhos",    125, 15.0f,  300,  8,  22.5f, { 150,  80,  40, 255 } },
    [TIPO_BORDER_COLLIE] = { "Border Collie", "Ataca 3 linhas",   300,  7.5f,  300,  7, -90.0f, {  70,  70,  86, 255 } },
    [TIPO_ROTTWEILER]    = { "Rottweiler",    "Explosão 3x3",     150, 35.0f,  300,  3,  90.0f, {  36,  32,  34, 255 } },
};

/* cachorro_info: retorna um ponteiro para os dados fixos do tipo na tabela. */
const InfoCachorro *cachorro_info(TipoCachorro tipo)
{
    return &infos[tipo];
}

/* cachorro_criar: aloca um cachorro com malloc, com a vida cheia do tipo e a
 * posição na matriz. O pote já começa contando o tempo do 1º petisco.
 * Retorna o ponteiro, ou NULL se faltar memória. */
Cachorro *cachorro_criar(TipoCachorro tipo, int linha, int coluna)
{
    Cachorro *cachorro = malloc(sizeof(Cachorro));
    if (cachorro == NULL) return NULL;

    cachorro->tipo = tipo;
    cachorro->linha = linha;
    cachorro->coluna = coluna;
    cachorro->vida = infos[tipo].vidaMaxima;
    cachorro->timer = (tipo == TIPO_POTE_RACAO) ? POTE_PRIMEIRA_PRODUCAO : 0.0f;
    return cachorro;
}

/* cachorro_destruir: libera a memória do cachorro (aceita NULL). */
void cachorro_destruir(Cachorro *cachorro)
{
    free(cachorro);
}

/* cachorro_desenhar: desenha o tipo como forma geométrica centrada em
 * 'centro', com tamanho 'raio' e contorno escuro. O círculo é um polígono de
 * 32 lados. O Pote de Ração é uma tigela: meia-lua vermelha com a ração por
 * cima. 'tinta' multiplica as cores (WHITE = normal; transparente = prévia). */
void cachorro_desenhar(TipoCachorro tipo, Vector2 centro, float raio, Color tinta)
{
    const InfoCachorro *info = &infos[tipo];
    Color cor = ColorTint(info->cor, tinta);

    if (tipo == TIPO_POTE_RACAO) {
        DrawCircleV((Vector2){ centro.x, centro.y - 2 }, raio * 0.75f, ColorTint((Color){ 150, 96, 50, 255 }, tinta));
        DrawCircleSector(centro, raio, 0, 180, 16, cor);
        return;
    }

    int lados = (info->lados == 0) ? 32 : info->lados;
    DrawPoly(centro, lados, raio, info->giro, cor);
    DrawPolyLinesEx(centro, lados, raio, info->giro, 2, ColorTint(ColorBrightness(info->cor, -0.5f), tinta));
}
