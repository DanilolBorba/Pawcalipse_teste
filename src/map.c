/* map.c - Campo de batalha (matriz 4x8), a casa e a rua. */

#include "map.h"
#include "config.h"
#include "ui.h"

#include <stddef.h>

#define FIM_MAPA_X  (MAPA_X + COLUNAS_MAPA * TAMANHO_CELULA)
#define FIM_MAPA_Y  (MAPA_Y + LINHAS_MAPA * TAMANHO_CELULA)

static const Color GRAMA_CLARA  = { 112, 136,  64, 255 };
static const Color GRAMA_ESCURA = {  98, 120,  56, 255 };
static const Color TELHADO      = { 122,  48,  42, 255 };
static const Color PAREDE       = { 184, 156, 120, 255 };
static const Color JANELA       = { 255, 214, 120, 255 };
static const Color PORTA        = {  88,  58,  36, 255 };
static const Color ASFALTO      = {  62,  58,  66, 255 };
static const Color TERRA        = {  84,  62,  46, 255 };

/* mapa_iniciar: marca todas as células da matriz como vazias (NULL). */
void mapa_iniciar(Mapa *mapa)
{
    for (int l = 0; l < LINHAS_MAPA; l++) {
        for (int c = 0; c < COLUNAS_MAPA; c++) mapa->grade[l][c] = NULL;
    }
}

/* mapa_limpar: percorre a matriz removendo (e liberando) todos os cachorros. */
void mapa_limpar(Mapa *mapa)
{
    for (int l = 0; l < LINHAS_MAPA; l++) {
        for (int c = 0; c < COLUNAS_MAPA; c++) mapa_remover(mapa, l, c);
    }
}

/* mapa_celula_no_ponto: converte um ponto da tela na linha e na coluna da
 * célula, devolvidas pelos ponteiros. Retorna false se o ponto estiver fora
 * do campo (nesse caso os ponteiros não são alterados). */
bool mapa_celula_no_ponto(Vector2 ponto, int *linha, int *coluna)
{
    if (ponto.x < MAPA_X || ponto.x >= FIM_MAPA_X || ponto.y < MAPA_Y || ponto.y >= FIM_MAPA_Y) return false;

    *linha = (int)(ponto.y - MAPA_Y) / TAMANHO_CELULA;
    *coluna = (int)(ponto.x - MAPA_X) / TAMANHO_CELULA;
    return true;
}

/* mapa_retangulo_celula: retorna a área (x, y, largura, altura) da célula. */
Rectangle mapa_retangulo_celula(int linha, int coluna)
{
    return (Rectangle){ MAPA_X + coluna * TAMANHO_CELULA, MAPA_Y + linha * TAMANHO_CELULA, TAMANHO_CELULA, TAMANHO_CELULA };
}

/* mapa_centro_celula: retorna o ponto central da célula. */
Vector2 mapa_centro_celula(int linha, int coluna)
{
    Rectangle r = mapa_retangulo_celula(linha, coluna);
    return (Vector2){ r.x + TAMANHO_CELULA / 2, r.y + TAMANHO_CELULA / 2 };
}

/* mapa_colocar: guarda o ponteiro do cachorro na célula indicada pelos campos
 * linha/coluna dele. Retorna false, sem colocar, se a célula estiver ocupada. */
bool mapa_colocar(Mapa *mapa, Cachorro *cachorro)
{
    if (mapa->grade[cachorro->linha][cachorro->coluna] != NULL) return false;
    mapa->grade[cachorro->linha][cachorro->coluna] = cachorro;
    return true;
}

/* mapa_remover: libera o cachorro da célula (free de NULL é seguro, então
 * funciona mesmo com a célula vazia) e marca a célula como livre. */
void mapa_remover(Mapa *mapa, int linha, int coluna)
{
    cachorro_destruir(mapa->grade[linha][coluna]);
    mapa->grade[linha][coluna] = NULL;
}

/* mapa_mover: passa o ponteiro do cachorro da célula atual para a célula de
 * destino e atualiza a posição guardada nele (vida e timer continuam os
 * mesmos). Retorna false, sem mover, se o destino estiver ocupado. */
bool mapa_mover(Mapa *mapa, Cachorro *cachorro, int linha, int coluna)
{
    if (mapa->grade[linha][coluna] != NULL) return false;

    mapa->grade[cachorro->linha][cachorro->coluna] = NULL;
    cachorro->linha = linha;
    cachorro->coluna = coluna;
    mapa->grade[linha][coluna] = cachorro;
    return true;
}

/* desenhar_cachorro_no_campo: desenha o cachorro no centro da sua célula,
 * no quadro atual da animação (o Chihuahua fica escuro até a mina armar)
 * e, se ele estiver ferido, a barra de vida logo acima. */
static void desenhar_cachorro_no_campo(const Cachorro *cachorro)
{
    Vector2 centro = mapa_centro_celula(cachorro->linha, cachorro->coluna);
    int vidaMaxima = cachorro_info(cachorro->tipo)->vidaMaxima;
    bool desarmada = (cachorro->tipo == TIPO_CHIHUAHUA && cachorro->timer > 0.0f);

    cachorro_desenhar(cachorro->tipo, centro, RAIO_CACHORRO, cachorro_quadro(cachorro), desarmada ? GRAY : WHITE);
    if (cachorro->vida < vidaMaxima) ui_desenhar_barra_vida((Vector2){ centro.x, centro.y - 28 }, 30, (float)cachorro->vida / vidaMaxima);
}

/* mapa_desenhar: desenha o gramado em xadrez, a casa à esquerda (telhado,
 * parede, janela acesa, porta e a placa "LAR"), a rua à direita, a terra
 * embaixo e, por fim, cada cachorro da matriz no centro da sua célula. */
void mapa_desenhar(const Mapa *mapa)
{
    for (int l = 0; l < LINHAS_MAPA; l++) {
        for (int c = 0; c < COLUNAS_MAPA; c++) {
            DrawRectangleRec(mapa_retangulo_celula(l, c), ((l + c) % 2 == 0) ? GRAMA_CLARA : GRAMA_ESCURA);
        }
    }

    DrawRectangle(0, MAPA_Y - 16, MAPA_X, 16, TELHADO);
    DrawRectangle(0, MAPA_Y, MAPA_X, FIM_MAPA_Y - MAPA_Y, PAREDE);
    DrawRectangle(14, 120, 22, 20, JANELA);
    DrawRectangle(18, 270, 28, 50, PORTA);
    ui_texto_centro("LAR", 32, 254, FONTE_PEQUENA, COR_PETISCO);
    DrawRectangle(FIM_MAPA_X, 68, LARGURA_VIRTUAL - FIM_MAPA_X, ALTURA_VIRTUAL - 68, ASFALTO);
    DrawRectangle(0, FIM_MAPA_Y, LARGURA_VIRTUAL, ALTURA_VIRTUAL - FIM_MAPA_Y, TERRA);

    for (int l = 0; l < LINHAS_MAPA; l++) {
        for (int c = 0; c < COLUNAS_MAPA; c++) {
            if (mapa->grade[l][c] != NULL) desenhar_cachorro_no_campo(mapa->grade[l][c]);
        }
    }
}
