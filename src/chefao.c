/* chefao.c - O Rei dos Gatos Zumbis: hordas, invocações, bola de pelo e desenho. */

#include "chefao.h"
#include "config.h"
#include "horda.h"
#include "ui.h"

#include <math.h>
#include <stdlib.h>

#define PREPARACAO_CHEFAO  12.0f   // segundos antes da 1ª invocação
#define PRIMEIRA_BOLA      30.0f   // segundos até a 1ª bola de pelo
#define INTERVALO_BOLA     20.0f
#define VELOCIDADE_BOLA    110.0f  // pixels por segundo
#define DURACAO_ANUNCIO    3.0f
#define TEMPO_COR_DANO     0.12f
#define SAIDA_GATOS_X      (FRENTE_CHEFAO_X + 8)   // onde os gatos invocados aparecem
#define ESCALA_CHEFAO      2.5f
#define CHEFAO_X           (MAPA_X + COLUNA_CHEFAO * TAMANHO_CELULA + 60)   // pés do Rei
#define CHEFAO_Y           (MAPA_Y + LINHAS_MAPA * TAMANHO_CELULA / 2 + 52)
#define ALTURA_COROA       6

typedef struct HordaChefao {
    int   gatos;                        // gatos por invocação
    float intervalo;                    // segundos entre duas invocações
    int   chances[TOTAL_TIPOS_GATO];    // peso de cada tipo de gato no sorteio
} HordaChefao;

static const HordaChefao hordas[HORDAS_POR_FASE] = {
    /*                padrão caixa lata siamês persa telhado maine */
    { 2, 9.0f, {         6,    2,   0,    0,     0,    0,     0 } },   // 1ª: fácil
    { 2, 8.0f, {         4,    2,   1,    2,     0,    1,     0 } },   // 2ª: intermediária
    { 3, 8.0f, {         3,    2,   2,    1,     2,    1,     0 } },   // 3ª: intermediária
    { 3, 6.0f, {         3,    2,   2,    2,     1,    1,     1 } },   // 4ª: difícil
};

static const Color TINTA_CHEFAO = { 200, 170, 225, 255 };   // pelo arroxeado do Rei
static const Color COR_BOLA     = { 150, 130, 160, 255 };

static const char *COROA[ALTURA_COROA] = {
    "#...#...#",
    "##.#y#.##",
    "#y#yyy#y#",
    "#yyyyyyy#",
    "#yryyyry#",
    "#########",
};
static const char SIMBOLOS_COROA[] = "#yr";
static const Color CORES_COROA[] = {
    {  60,  36,  10, 255 },   // # contorno
    { 246, 196,  60, 255 },   // y ouro
    { 200,  40,  40, 255 },   // r rubi
};

/* chefao_criar: aloca o Rei com malloc, com a vida cheia, na 1ª horda e
 * sem bola rolando; o anúncio da chegada dele já começa na tela. Retorna
 * NULL se faltar memória. */
Chefao *chefao_criar(void)
{
    Chefao *chefao = malloc(sizeof(Chefao));
    if (chefao == NULL) return NULL;

    *chefao = (Chefao){ 0 };
    chefao->vida = VIDA_CHEFAO;
    chefao->timerInvocar = PREPARACAO_CHEFAO;
    chefao->timerBola = PRIMEIRA_BOLA;
    chefao->timerAnuncio = DURACAO_ANUNCIO;
    chefao->bolaLinha = -1;
    return chefao;
}

/* chefao_destruir: libera a memória do chefão (aceita NULL). */
void chefao_destruir(Chefao *chefao)
{
    free(chefao);
}

/* sortear_gato: sorteia o tipo de um gato invocado, com chance
 * proporcional ao peso de cada tipo na horda atual. */
static TipoGato sortear_gato(const HordaChefao *horda)
{
    int soma = 0;
    for (int t = 0; t < TOTAL_TIPOS_GATO; t++) soma += horda->chances[t];

    int sorteio = rand() % soma;
    int tipo = 0;
    while (sorteio >= horda->chances[tipo]) {
        sorteio -= horda->chances[tipo];
        tipo++;
    }
    return (TipoGato)tipo;
}

/* invocar_gatos: põe na lista os gatos de uma invocação da horda atual,
 * cada um numa linha aleatória, saindo da frente do Rei (um pouco
 * espaçados, para não ficarem um em cima do outro). */
static void invocar_gatos(Chefao *chefao, Gato **gatos)
{
    const HordaChefao *horda = &hordas[chefao->horda];
    for (int i = 0; i < horda->gatos; i++) {
        if (gato_adicionar(gatos, sortear_gato(horda), rand() % LINHAS_MAPA)) (*gatos)->x = SAIDA_GATOS_X + i * 12;
    }
    chefao->timerInvocar = horda->intervalo;
}

/* trocar_horda: a horda vem da vida já perdida (uma nova a cada quarto).
 * Ao mudar, mostra o anúncio e o Rei invoca gatos na hora. */
static void trocar_horda(Chefao *chefao)
{
    int horda = (VIDA_CHEFAO - chefao->vida) * HORDAS_POR_FASE / VIDA_CHEFAO;
    if (horda >= HORDAS_POR_FASE) horda = HORDAS_POR_FASE - 1;
    if (horda <= chefao->horda) return;

    chefao->horda = horda;
    chefao->timerAnuncio = DURACAO_ANUNCIO;
    chefao->timerInvocar = 0.0f;
}

/* sortear_linha_com_cachorro: sorteia uma das linhas que têm pelo menos um
 * cachorro. Retorna -1 se o campo estiver vazio. */
static int sortear_linha_com_cachorro(const Mapa *mapa)
{
    int linhas[LINHAS_MAPA];
    int total = 0;
    for (int l = 0; l < LINHAS_MAPA; l++) {
        for (int c = 0; c < COLUNAS_MAPA; c++) {
            if (mapa->grade[l][c] == NULL) continue;
            linhas[total++] = l;
            break;
        }
    }
    return (total > 0) ? linhas[rand() % total] : -1;
}

/* rolar_bola: a bola de pelo rola para a esquerda; ao entrar na célula de
 * um cachorro (o mais à direita da linha), o derruba e some. Se passar do
 * campo sem achar ninguém, também some. */
static void rolar_bola(Chefao *chefao, Mapa *mapa, float dt)
{
    int linha, coluna;
    chefao->bolaX -= VELOCIDADE_BOLA * dt;
    Vector2 ponto = { chefao->bolaX, mapa_centro_celula(chefao->bolaLinha, 0).y };

    if (!mapa_celula_no_ponto(ponto, &linha, &coluna)) chefao->bolaLinha = -1;
    else if (mapa->grade[linha][coluna] != NULL) {
        mapa_remover(mapa, linha, coluna);
        chefao->bolaLinha = -1;
    }
}

/* chefao_atualizar: conta os tempos do Rei, troca de horda conforme a vida,
 * invoca gatos a cada intervalo da horda e, a cada INTERVALO_BOLA segundos,
 * rola uma bola de pelo numa linha que tenha cachorros. */
void chefao_atualizar(Chefao *chefao, Gato **gatos, Mapa *mapa, float dt)
{
    chefao->animacao += dt;
    if (chefao->timerDano > 0.0f) chefao->timerDano -= dt;
    if (chefao->timerAnuncio > 0.0f) chefao->timerAnuncio -= dt;
    trocar_horda(chefao);

    chefao->timerInvocar -= dt;
    if (chefao->timerInvocar <= 0.0f) invocar_gatos(chefao, gatos);

    if (chefao->bolaLinha >= 0) rolar_bola(chefao, mapa, dt);
    chefao->timerBola -= dt;
    if (chefao->timerBola > 0.0f) return;
    chefao->timerBola = INTERVALO_BOLA;
    chefao->bolaLinha = sortear_linha_com_cachorro(mapa);
    chefao->bolaX = FRENTE_CHEFAO_X;
}

/* chefao_ferir: tira 'dano' da vida (sem passar de zero) e deixa o Rei
 * avermelhado por um instante. Dano zero não faz nada. */
void chefao_ferir(Chefao *chefao, int dano)
{
    if (dano <= 0) return;
    chefao->vida = (chefao->vida > dano) ? chefao->vida - dano : 0;
    chefao->timerDano = TEMPO_COR_DANO;
}

/* chefao_derrotado: retorna true quando a vida do Rei acabou. */
bool chefao_derrotado(const Chefao *chefao)
{
    return chefao->vida <= 0;
}

/* chefao_fracao_vida: quanto da vida ainda resta, de 0 (derrotado) a 1. */
float chefao_fracao_vida(const Chefao *chefao)
{
    return (float)chefao->vida / VIDA_CHEFAO;
}

/* chefao_anuncio: enquanto o anúncio estiver na tela, retorna o texto da
 * horda atual (a chegada do Rei, a 2ª, a 3ª ou a última); senão, NULL. */
const char *chefao_anuncio(const Chefao *chefao)
{
    if (chefao->timerAnuncio <= 0.0f) return NULL;
    if (chefao->horda == 0) return "O REI CHEGOU!";
    if (chefao->horda == HORDAS_POR_FASE - 1) return "ÚLTIMA HORDA!";
    return TextFormat("HORDA %d!", chefao->horda + 1);
}

/* desenhar_bola: a bola de pelo, com três tufos que giram enquanto ela rola. */
static void desenhar_bola(const Chefao *chefao)
{
    Vector2 centro = { chefao->bolaX, mapa_centro_celula(chefao->bolaLinha, 0).y + 14 };
    DrawCircleV(centro, 10, COR_BOLA);
    for (int i = 0; i < 3; i++) {
        float angulo = chefao->bolaX / 10.0f + i * 2.1f;
        DrawLineEx(centro, (Vector2){ centro.x + cosf(angulo) * 9, centro.y + sinf(angulo) * 9 }, 2, Fade(COR_BORDA, 0.6f));
    }
    DrawCircleLinesV(centro, 10, COR_BORDA);
}

/* chefao_desenhar: escurece de roxo a última coluna e a rua (o território
 * do Rei) e desenha o Rei: o sprite do gato zumbi ampliado e arroxeado
 * (avermelhado logo depois de levar dano), subindo e descendo devagar como
 * se respirasse, com a coroa em pixel-art na cabeça. Por fim, a bola de
 * pelo, se houver uma rolando. */
void chefao_desenhar(const Chefao *chefao)
{
    Rectangle territorio = mapa_retangulo_celula(0, COLUNA_CHEFAO);
    Vector2 pes = { CHEFAO_X, CHEFAO_Y + sinf(chefao->animacao * 2.0f) * 2.0f };
    Color tinta = (chefao->timerDano > 0.0f) ? (Color){ 255, 110, 110, 255 } : TINTA_CHEFAO;

    DrawRectangle(territorio.x, territorio.y, LARGURA_VIRTUAL - territorio.x, LINHAS_MAPA * TAMANHO_CELULA, Fade((Color){ 60, 20, 80, 255 }, 0.35f));
    DrawEllipse(CHEFAO_X + 10, CHEFAO_Y, 16 * ESCALA_CHEFAO, 4 * ESCALA_CHEFAO, Fade(BLACK, 0.3f));
    if (sprite_existe(SPRITE_GATO_PADRAO)) sprite_desenhar(SPRITE_GATO_PADRAO, 0, pes, ESCALA_CHEFAO, tinta);
    else                                   DrawCircleV((Vector2){ pes.x, pes.y - 50 }, 50, tinta);
    ui_desenhar_pixel_art(COROA, ALTURA_COROA, (Vector2){ pes.x - 33, pes.y - 111 }, 3, SIMBOLOS_COROA, CORES_COROA);

    if (chefao->bolaLinha >= 0) desenhar_bola(chefao);
}

/* chefao_desenhar_vida: na área da barra superior, o nome do Rei e a barra
 * de vida, dividida em 4 partes (uma por horda), com a vida em números. */
void chefao_desenhar_vida(const Chefao *chefao, Rectangle area)
{
    Rectangle barra = { area.x, area.y + 28, area.width, 14 };
    ui_texto("REI DOS GATOS ZUMBIS", area.x, area.y + 4, FONTE_MEDIA, COR_ALERTA);

    DrawRectangleRec(barra, COR_SOMBRA);
    DrawRectangle(barra.x, barra.y, barra.width * chefao_fracao_vida(chefao), barra.height, COR_SANGUE);
    for (int h = 1; h < HORDAS_POR_FASE; h++) DrawRectangle(barra.x + barra.width * h / HORDAS_POR_FASE, barra.y, 2, barra.height, COR_BORDA);
    DrawRectangleLinesEx(barra, 2, COR_BORDA);
    ui_texto_centro(TextFormat("%d / %d", chefao->vida, VIDA_CHEFAO), barra.x + barra.width / 2, barra.y + 2, FONTE_PEQUENA, COR_TEXTO);
}
