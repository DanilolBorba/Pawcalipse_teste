/* cenario.c - Cenário do menu (pôr do sol sobre a cidade em ruínas) e o
 * título do jogo. Os detalhes "aleatórios" usam cenario_ruido, que sempre
 * devolve o mesmo número para a mesma entrada, então nada muda de lugar
 * de um frame para o outro. */

#include "cenario.h"
#include "config.h"
#include "ui.h"

#include <math.h>

#define HORIZONTE_MENU  300   // linha onde os prédios encostam no chão

static const Color CEU_TOPO       = {  52,  34,  88, 255 };
static const Color CEU_MEIO       = { 186,  72, 102, 255 };
static const Color CEU_HORIZONTE  = { 252, 160,  76, 255 };
static const Color SOL_CENTRO     = { 255, 232, 130, 255 };
static const Color SOL_BORDA      = { 255, 168,  80, 255 };
static const Color NUVEM          = { 204,  96, 122, 255 };
static const Color NUVEM_BRILHO   = { 255, 182, 120, 255 };
static const Color PASSARO        = {  44,  24,  44, 255 };
static const Color PREDIO_LONGE   = { 120,  66,  98, 255 };
static const Color JANELA_ESCURA  = {  40,  34,  46, 255 };
static const Color JANELA_ACESA   = { 255, 214, 120, 255 };
static const Color JANELA_VIDRO   = { 190, 200, 214, 255 };
static const Color TABUA          = { 124,  86,  52, 255 };
static const Color FERRO          = {  52,  46,  54, 255 };
static const Color LUZ_ANTENA     = { 255,  48,  48, 255 };
static const Color LUZ_APAGADA    = {  96,  24,  24, 255 };
static const Color CAIXA_DAGUA    = {  86,  62,  48, 255 };
static const Color TERRA          = {  78,  58,  44, 255 };
static const Color TERRA_ESCURA   = {  56,  40,  32, 255 };
static const Color PEDRA          = { 132, 108,  86, 255 };
static const Color GRAMA_SECA     = { 118, 116,  58, 255 };
static const Color GRAMA_ESCURA   = {  80,  86,  44, 255 };
static const Color ASFALTO        = {  56,  52,  60, 255 };
static const Color RACHADURA      = {  34,  30,  38, 255 };
static const Color MEIO_FIO       = { 116, 106, 100, 255 };
static const Color FAIXA_RUA      = { 196, 164,  84, 255 };
static const Color BARRIL         = { 116,  60,  40, 255 };
static const Color FOGO_FORA      = { 255, 116,  40, 255 };
static const Color FOGO_MEIO      = { 255, 196,  72, 255 };
static const Color FOGO_DENTRO    = { 255, 246, 200, 255 };

enum { TELHADO_RETO, TELHADO_QUEBRADO, TELHADO_CAIXA_DAGUA };

typedef struct Predio {
    int   x, largura, altura;
    Color cor;       // tons neutros: concreto, bege, cinza-azulado...
    int   telhado;
    bool  antena;    // antena com luz vermelha piscando
} Predio;

static const Predio predios[] = {
    {  -6, 58, 112, {  86, 80, 86, 255 }, TELHADO_QUEBRADO,    false },
    {  48, 42, 156, { 104, 92, 82, 255 }, TELHADO_RETO,        true  },
    {  94, 62,  88, {  74, 76, 88, 255 }, TELHADO_CAIXA_DAGUA, false },
    { 152, 46, 176, {  96, 86, 76, 255 }, TELHADO_RETO,        true  },
    { 202, 56, 106, {  82, 72, 70, 255 }, TELHADO_QUEBRADO,    false },
    { 262, 38, 132, {  86, 80, 86, 255 }, TELHADO_RETO,        true  },
    { 304, 70,  82, { 104, 92, 82, 255 }, TELHADO_CAIXA_DAGUA, false },
    { 378, 48, 148, {  74, 76, 88, 255 }, TELHADO_QUEBRADO,    false },
    { 430, 52,  92, {  96, 86, 76, 255 }, TELHADO_RETO,        true  },
    { 486, 40,  78, {  82, 72, 70, 255 }, TELHADO_CAIXA_DAGUA, false },
    { 530, 60, 120, {  86, 80, 86, 255 }, TELHADO_QUEBRADO,    true  },
    { 594, 52, 150, { 104, 92, 82, 255 }, TELHADO_RETO,        true  },
};
#define TOTAL_PREDIOS ((int)(sizeof(predios) / sizeof(predios[0])))

/* cenario_ruido: embaralha três inteiros e devolve um número pseudoaleatório.
 * A mesma entrada sempre gera o mesmo número, então serve para espalhar
 * detalhes (janelas, pedras, rachaduras) sem que eles mudem a cada frame. */
unsigned int cenario_ruido(int a, int b, int c)
{
    unsigned int h = (unsigned int)a * 374761393u
                   + (unsigned int)b * 668265263u
                   + (unsigned int)c * 2246822519u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

/* cor_do_ceu: retorna a cor do degradê do céu na altura y, para um céu que
 * vai de 0 até 'altura' (roxo no topo, rosa no meio, laranja no horizonte). */
static Color cor_do_ceu(float y, float altura)
{
    float f = y / altura;
    if (f < 0.0f) f = 0.0f;
    if (f > 1.0f) f = 1.0f;
    if (f < 0.5f) return ColorLerp(CEU_TOPO, CEU_MEIO, f / 0.5f);
    return ColorLerp(CEU_MEIO, CEU_HORIZONTE, (f - 0.5f) / 0.5f);
}

/* cenario_desenhar_ceu: desenha o céu do pôr do sol de y = 0 até 'altura':
 * degradê, estrelas piscando no topo, sol com brilho e faixas "retrô",
 * nuvens que se movem devagar e corvos voando. 'tempo' anima tudo. */
void cenario_desenhar_ceu(int altura, float tempo)
{
    int metade = altura / 2;
    DrawRectangleGradientV(0, 0, LARGURA_VIRTUAL, metade, CEU_TOPO, CEU_MEIO);
    DrawRectangleGradientV(0, metade, LARGURA_VIRTUAL, altura - metade, CEU_MEIO, CEU_HORIZONTE);

    for (int i = 0; i < 16; i++) {
        int x = (int)(cenario_ruido(i, 1, 0) % LARGURA_VIRTUAL);
        int y = (int)(cenario_ruido(i, 2, 0) % (unsigned int)(altura / 4 + 1));
        float brilho = 0.35f + 0.35f * sinf(tempo * 2.0f + i);
        DrawPixel(x, y, Fade(WHITE, brilho));
    }

    float raio = altura * 0.15f;
    Vector2 sol = { 500.0f, altura * 0.62f };
    DrawCircleGradient(sol, raio * 2.2f, Fade(SOL_BORDA, 0.35f), Fade(SOL_BORDA, 0.0f));
    DrawCircleV(sol, raio, SOL_BORDA);
    DrawCircleV(sol, raio - 3, SOL_CENTRO);
    for (int k = 0; k < 5; k++) {
        float y = sol.y + raio * (0.15f + k * 0.18f);
        int espessura = 1 + k;
        DrawRectangle((int)(sol.x - raio - 1), (int)y, (int)(raio * 2 + 2), espessura, cor_do_ceu(y, (float)altura));
    }

    static const float nuvens[][4] = {   // x inicial, y (fração da altura), largura, velocidade
        {  20, 0.22f, 70, 3.0f }, { 260, 0.14f, 46, 2.0f }, { 400, 0.36f, 90, 4.0f },
        { 560, 0.20f, 54, 2.5f }, { 150, 0.44f, 64, 3.5f },
    };
    for (int i = 0; i < 5; i++) {
        float largura = nuvens[i][2];
        int x = (int)(fmodf(nuvens[i][0] + tempo * nuvens[i][3], LARGURA_VIRTUAL + largura) - largura);
        int y = (int)(nuvens[i][1] * altura);
        DrawRectangle(x + (int)(largura * 0.2f), y - 3, (int)(largura * 0.5f), 3, NUVEM);
        DrawRectangle(x, y, (int)largura, 4, NUVEM);
        DrawRectangle(x + 4, y + 4, (int)largura - 8, 1, NUVEM_BRILHO);
    }

    for (int i = 0; i < 3; i++) {
        int x = (int)(fmodf(tempo * (12 + i * 4) + i * 230, LARGURA_VIRTUAL + 40) - 20);
        int y = (int)(altura * (0.18f + 0.08f * i) + sinf(tempo * 1.5f + i) * 4);
        int asa = ((int)(tempo * 6 + i) % 2) ? -2 : 1;
        DrawLine(x - 4, y + asa, x, y, PASSARO);
        DrawLine(x, y, x + 4, y + asa, PASSARO);
    }
}

/* cenario_desenhar_skyline_distante: desenha, apoiada na linha 'base', a
 * silhueta roxa dos prédios ao longe, com antenas nos mais altos e poucas
 * janelas acesas bem fracas. */
void cenario_desenhar_skyline_distante(int base)
{
    static const int distantes[][2] = {   // largura, altura
        { 40,  70 }, { 30,  96 }, { 52,  60 }, { 24, 118 }, { 46,  84 }, { 36,  74 }, { 60, 100 },
        { 28,  64 }, { 44, 128 }, { 50,  80 }, { 34,  92 }, { 58,  66 }, { 30, 104 }, { 48,  76 },
    };
    const int total = (int)(sizeof(distantes) / sizeof(distantes[0]));

    int x = -6;
    for (int i = 0; i < total; i++) {
        int largura = distantes[i][0], altura = distantes[i][1];
        DrawRectangle(x, base - altura, largura, altura, PREDIO_LONGE);
        if (altura > 100) {
            DrawLine(x + largura / 2, base - altura, x + largura / 2, base - altura - 10, PREDIO_LONGE);
        }
        for (int k = 0; k < 3; k++) {
            if (cenario_ruido(i, k, 40) % 3 != 0) continue;
            int jx = x + 4 + (int)(cenario_ruido(i, k, 41) % (unsigned int)(largura - 8));
            int jy = base - altura + 6 + (int)(cenario_ruido(i, k, 42) % (unsigned int)(altura - 12));
            DrawRectangle(jx, jy, 2, 2, Fade(JANELA_ACESA, 0.45f));
        }
        x += largura + 5;
    }
}

/* desenhar_janela: desenha uma janela em (jx, jy). O ruído do prédio, da
 * coluna e da linha decide o tipo: acesa (algumas piscam, como energia
 * falhando), quebrada com cacos de vidro, tapada com tábuas ou apagada. */
static void desenhar_janela(int jx, int jy, int predio, int coluna, int linha, float tempo)
{
    unsigned int sorte = cenario_ruido(predio * 31 + coluna, linha, 7) % 100;

    if (sorte < 26) {
        bool falhando = (cenario_ruido(predio, coluna, linha) % 7 == 0)
                        && sinf(tempo * 9.0f + coluna * 3 + linha) > 0.75f;
        if (falhando) {
            DrawRectangle(jx, jy, 4, 6, JANELA_ESCURA);
        } else {
            DrawRectangle(jx - 1, jy - 1, 6, 8, Fade(JANELA_ACESA, 0.25f));
            DrawRectangle(jx, jy, 4, 6, JANELA_ACESA);
            DrawRectangle(jx, jy + 3, 4, 1, Fade(TABUA, 0.5f));
        }
    } else if (sorte < 40) {
        DrawRectangle(jx, jy, 4, 6, JANELA_ESCURA);
        DrawPixel(jx, jy, JANELA_VIDRO);
        DrawPixel(jx + 1, jy, JANELA_VIDRO);
        DrawPixel(jx, jy + 1, JANELA_VIDRO);
        DrawPixel(jx + 3, jy + 5, JANELA_VIDRO);
        DrawPixel(jx + 2, jy + 3, Fade(JANELA_VIDRO, 0.5f));
    } else if (sorte < 47) {
        DrawRectangle(jx - 1, jy, 6, 6, JANELA_ESCURA);
        DrawRectangle(jx - 1, jy + 1, 6, 2, TABUA);
        DrawRectangle(jx - 1, jy + 4, 6, 1, TABUA);
    } else {
        DrawRectangle(jx, jy, 4, 6, JANELA_ESCURA);
    }
}

/* desenhar_predio: desenha um prédio do primeiro plano apoiado em 'base':
 * corpo (com topo desabado e vergalhões se for quebrado), sombra na
 * lateral esquerda e luz do sol na direita, andares com janelas, porta de
 * aço no térreo, rachadura em alguns, caixa d'água e antena piscando. */
static void desenhar_predio(const Predio *p, int indice, int base, float tempo)
{
    int topo = base - p->altura;
    Color sombra = ColorBrightness(p->cor, -0.3f);
    Color luz = ColorLerp(p->cor, SOL_BORDA, 0.3f);
    int inicioCorpo = topo;

    if (p->telhado == TELHADO_QUEBRADO) {
        inicioCorpo = topo + 10;
        DrawRectangle(p->x, inicioCorpo, p->largura, p->altura - 10, p->cor);
        for (int bx = 0; bx < p->largura; bx += 4) {
            int queda = (int)(cenario_ruido(indice, bx, 3) % 11);
            int largura = (bx + 4 > p->largura) ? p->largura - bx : 4;
            DrawRectangle(p->x + bx, topo + queda, largura, 11 - queda, p->cor);
            if (cenario_ruido(indice, bx, 4) % 6 == 0) {
                DrawLine(p->x + bx + 1, topo + queda, p->x + bx + 3, topo + queda - 5, FERRO);
            }
        }
    } else {
        DrawRectangle(p->x, topo, p->largura, p->altura, p->cor);
        DrawRectangle(p->x - 1, topo, p->largura + 2, 3, sombra);
    }

    DrawRectangle(p->x, inicioCorpo, 2, base - inicioCorpo, sombra);
    DrawRectangle(p->x + p->largura - 2, inicioCorpo, 2, base - inicioCorpo, luz);

    int colunas = (p->largura - 6) / 8;
    int linhas = (base - inicioCorpo - 18) / 12;
    int margemX = (p->largura - (colunas * 8 - 4)) / 2;
    for (int l = 0; l < linhas; l++) {
        int jy = inicioCorpo + 8 + l * 12;
        DrawRectangle(p->x + 2, jy + 8, p->largura - 4, 1, ColorBrightness(p->cor, -0.15f));
        for (int c = 0; c < colunas; c++) {
            desenhar_janela(p->x + margemX + c * 8, jy, indice, c, l, tempo);
        }
    }

    int px = p->x + p->largura / 2 - 6;
    DrawRectangle(px, base - 12, 12, 12, (Color){ 62, 60, 66, 255 });
    for (int k = 0; k < 4; k++) DrawRectangle(px, base - 11 + k * 3, 12, 1, (Color){ 44, 42, 48, 255 });

    if (indice % 3 == 1) {
        int rx = p->x + p->largura / 3, ry = inicioCorpo + 18;
        for (int k = 0; k < 6; k++) {
            int nx = rx + (int)(cenario_ruido(indice, k, 9) % 5) - 2;
            int ny = ry + 5;
            DrawLine(rx, ry, nx, ny, sombra);
            rx = nx;
            ry = ny;
        }
    }

    if (p->telhado == TELHADO_CAIXA_DAGUA) {
        int cx = p->x + 8;
        DrawLine(cx + 2, topo, cx + 2, topo - 4, FERRO);
        DrawLine(cx + 11, topo, cx + 11, topo - 4, FERRO);
        DrawRectangle(cx, topo - 14, 14, 10, CAIXA_DAGUA);
        DrawRectangle(cx, topo - 11, 14, 1, ColorBrightness(CAIXA_DAGUA, -0.3f));
        DrawRectangle(cx, topo - 7, 14, 1, ColorBrightness(CAIXA_DAGUA, -0.3f));
        DrawRectangle(cx + 2, topo - 17, 10, 3, ColorBrightness(CAIXA_DAGUA, -0.2f));
    }

    if (p->antena) {
        int ax = p->x + p->largura * 2 / 3;
        int at = topo + (p->telhado == TELHADO_QUEBRADO ? 4 : 0);
        DrawLine(ax, at, ax, at - 20, FERRO);
        DrawLine(ax - 4, at - 12, ax + 4, at - 12, FERRO);
        DrawLine(ax - 2, at - 17, ax + 2, at - 17, FERRO);
        bool acesa = fmodf(tempo + indice * 0.37f, 1.6f) < 0.8f;
        if (acesa) {
            DrawCircle(ax, at - 21, 4, Fade(LUZ_ANTENA, 0.3f));
            DrawRectangle(ax - 1, at - 22, 2, 2, LUZ_ANTENA);
        } else {
            DrawRectangle(ax - 1, at - 22, 2, 2, LUZ_APAGADA);
        }
    }
}

/* desenhar_fogo: desenha um barril enferrujado em (x, base) com fogo:
 * brilho no chão, chamas em colunas que oscilam com o tempo (laranja,
 * amarelo e quase branco no centro) e faíscas subindo. */
static void desenhar_fogo(int x, int base, float tempo)
{
    float tremor = 0.8f + 0.2f * sinf(tempo * 17.0f);
    DrawCircleGradient((Vector2){ x + 6.0f, base + 4.0f }, 30.0f * tremor, Fade(FOGO_FORA, 0.3f), Fade(FOGO_FORA, 0.0f));

    DrawRectangle(x, base + 2, 12, 16, BARRIL);
    DrawRectangle(x, base + 6, 12, 1, ColorBrightness(BARRIL, -0.35f));
    DrawRectangle(x, base + 13, 12, 1, ColorBrightness(BARRIL, -0.35f));
    DrawRectangle(x - 1, base + 1, 14, 2, ColorBrightness(BARRIL, -0.45f));
    DrawRectangle(x + 9, base + 3, 2, 14, ColorBrightness(BARRIL, 0.2f));

    for (int c = 0; c < 10; c++) {
        float formato = 1.0f - fabsf(c - 4.5f) / 6.0f;
        float h = (10.0f + 3.0f * sinf(tempo * 13.0f + c) + 2.0f * sinf(tempo * 7.3f + c * 2)) * formato;
        int ix = x + 1 + c;
        DrawRectangle(ix, base + 1 - (int)h, 1, (int)h, FOGO_FORA);
        if (c >= 2 && c <= 7) DrawRectangle(ix, base + 1 - (int)(h * 0.6f), 1, (int)(h * 0.6f), FOGO_MEIO);
        if (c >= 3 && c <= 6) DrawRectangle(ix, base + 1 - (int)(h * 0.3f), 1, (int)(h * 0.3f), FOGO_DENTRO);
    }

    for (int k = 0; k < 4; k++) {
        float subida = fmodf(tempo * 24.0f + k * 11.0f, 34.0f);
        int fx = x + 6 + (int)(sinf(tempo * 3.0f + k) * 4.0f);
        DrawPixel(fx, base - 8 - (int)subida, Fade(FOGO_MEIO, 1.0f - subida / 34.0f));
    }
}

/* desenhar_chao: desenha o chão do menu a partir da linha 'base': terra,
 * meio-fio, rua rachada com faixa apagada, pedrinhas, grama seca, pegadas
 * de pata, ossos, pneu velho, cerca quebrada, placa de perigo e o barril
 * com fogo. */
static void desenhar_chao(int base, float tempo)
{
    DrawRectangle(0, base, LARGURA_VIRTUAL, ALTURA_VIRTUAL - base, TERRA);
    DrawRectangle(0, base + 26, LARGURA_VIRTUAL, 2, MEIO_FIO);
    DrawRectangle(0, base + 28, LARGURA_VIRTUAL, ALTURA_VIRTUAL - base - 28, ASFALTO);

    for (int x = 0; x < LARGURA_VIRTUAL; x += 40) {
        if (cenario_ruido(x, 5, 0) % 4 != 0) {
            DrawRectangle(x + 8, base + 43, 18, 2, Fade(FAIXA_RUA, 0.45f));
        }
    }
    for (int k = 0; k < 9; k++) {
        int rx = (int)(cenario_ruido(k, 6, 0) % LARGURA_VIRTUAL);
        int ry = base + 31 + (int)(cenario_ruido(k, 7, 0) % 24);
        for (int s = 0; s < 3; s++) {
            int nx = rx + 3 + (int)(cenario_ruido(k, s, 8) % 4);
            int ny = ry + (int)(cenario_ruido(k, s, 9) % 5) - 2;
            DrawLine(rx, ry, nx, ny, RACHADURA);
            rx = nx;
            ry = ny;
        }
    }

    for (int k = 0; k < 70; k++) {
        int px = (int)(cenario_ruido(k, 10, 0) % LARGURA_VIRTUAL);
        int py = base + 5 + (int)(cenario_ruido(k, 11, 0) % 20);
        DrawRectangle(px, py, 1 + (int)(k % 2), 1, (k % 3) ? PEDRA : TERRA_ESCURA);
    }

    DrawRectangle(0, base, LARGURA_VIRTUAL, 3, GRAMA_ESCURA);
    for (int x = 0; x < LARGURA_VIRTUAL; x += 2) {
        int h = (int)(cenario_ruido(x, 12, 0) % 6);
        if (h > 1) DrawRectangle(x, base + 1 - h, 1, h, (x % 4) ? GRAMA_SECA : GRAMA_ESCURA);
    }

    for (int k = 0; k < 9; k++) {
        int px = 130 + k * 26, py = base + 10 + (k % 2) * 6;
        DrawRectangle(px, py, 3, 2, TERRA_ESCURA);
        DrawPixel(px - 1, py - 2, TERRA_ESCURA);
        DrawPixel(px + 1, py - 3, TERRA_ESCURA);
        DrawPixel(px + 3, py - 2, TERRA_ESCURA);
    }

    ui_desenhar_osso((Vector2){ 214, base + 18.0f }, 0.5f, COR_OSSO);
    ui_desenhar_osso((Vector2){ 474, base + 13.0f }, 0.5f, COR_OSSO);
    DrawCircle(98, base + 15, 6, (Color){ 30, 28, 32, 255 });
    DrawCircle(98, base + 15, 2, TERRA);

    static const int alturaPostes[] = { 18, 16, 9, 17 };
    for (int k = 0; k < 4; k++) {
        DrawRectangle(6 + k * 16, base + 4 - alturaPostes[k], 3, alturaPostes[k], TABUA);
    }
    DrawRectangle(6, base - 10, 34, 2, TABUA);
    DrawRectangle(6, base - 4, 34, 2, TABUA);
    DrawLineEx((Vector2){ 40, base - 6.0f }, (Vector2){ 58, base + 1.0f }, 2, TABUA);

    DrawRectangle(560, base - 22, 2, 26, (Color){ 80, 80, 86, 255 });
    DrawRectangle(553, base - 31, 16, 12, (Color){ 222, 182, 48, 255 });
    DrawRectangleLines(553, base - 31, 16, 12, (Color){ 40, 30, 20, 255 });
    DrawRectangle(560, base - 29, 2, 5, (Color){ 40, 30, 20, 255 });
    DrawRectangle(560, base - 23, 2, 2, (Color){ 40, 30, 20, 255 });

    desenhar_fogo(600, base, tempo);
}

/* cenario_desenhar_menu: desenha o cenário completo das telas de menu, de
 * trás para a frente: céu, prédios distantes, prédios próximos e chão. */
void cenario_desenhar_menu(float tempo)
{
    cenario_desenhar_ceu(HORIZONTE_MENU, tempo);
    cenario_desenhar_skyline_distante(HORIZONTE_MENU);
    for (int i = 0; i < TOTAL_PREDIOS; i++) {
        desenhar_predio(&predios[i], i, HORIZONTE_MENU, tempo);
    }
    desenhar_chao(HORIZONTE_MENU, tempo);
}

/* titulo_desenhar: escreve "PawPocalypse" centralizado na altura y, em verde
 * com sombra vermelho-escura. O topo das letras é pintado de vermelho
 * (redesenhando o texto só numa faixa estreita, com BeginScissorMode) e de
 * algumas letras escorre sangue, com uma gota caindo em loop. */
void titulo_desenhar(int y, float tempo)
{
    static const int letrasComGota[] = { 0, 3, 7, 10 };   // P, P, l, s
    int largura = MeasureText(TITULO_JOGO, FONTE_GRANDE);
    int x = (LARGURA_VIRTUAL - largura) / 2;

    DrawText(TITULO_JOGO, x + 3, y + 3, FONTE_GRANDE, COR_SANGUE_ESC);
    DrawText(TITULO_JOGO, x, y, FONTE_GRANDE, COR_INFECTADO);
    BeginScissorMode(x, y, largura, 14);
    DrawText(TITULO_JOGO, x, y, FONTE_GRANDE, COR_SANGUE);
    EndScissorMode();

    for (int i = 0; i < 4; i++) {
        int letra = letrasComGota[i];
        int gx = x + 1 + (letra > 0 ? MeasureText(TextSubtext(TITULO_JOGO, 0, letra), FONTE_GRANDE) + 4 : 0);
        float queda = fmodf(tempo * 18.0f + i * 7.0f, 24.0f);
        DrawRectangle(gx, y + 28, 2, 6 + i % 2 * 3, COR_SANGUE);
        DrawRectangle(gx, y + 36 + (int)queda, 2, 3, Fade(COR_SANGUE, 1.0f - queda / 24.0f));
    }
}
