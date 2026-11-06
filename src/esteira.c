/* esteira.c - A esteira de cartas grátis da fase do chefão. */

#include "esteira.h"
#include "config.h"
#include "ui.h"

#include <math.h>
#include <stdlib.h>

#define INTERVALO_ESTEIRA  4.0f    // segundos entre duas cartas
#define VELOCIDADE_CARTA   60.0f   // pixels por segundo que as cartas sobem
#define ESTEIRA_TOPO       72      // y da vaga de cima
#define PASSO_VAGA         40      // distância entre duas vagas
#define ESPACO_LISTRAS     12      // distância entre as listras da faixa

static const Rectangle FAIXA = { 2, 68, 60, 290 };
static const Color COR_ESTEIRA = { 52, 48, 56, 255 };

/* Chance de cada raça vir na esteira (o Pote de Ração não vem: não há petiscos). */
static const int pesos[TOTAL_TIPOS_CACHORRO] = {
    [TIPO_BEAGLE] = 5, [TIPO_BULLDOG] = 4, [TIPO_DALMATA] = 3, [TIPO_HUSKY] = 3,
    [TIPO_PASTOR_ALEMAO] = 2, [TIPO_WESTIE] = 2, [TIPO_SAO_BERNARDO] = 1,
    [TIPO_DACHSHUND] = 2, [TIPO_ROTTWEILER] = 1, [TIPO_SHIBA] = 2,
};

/* sortear_raca: sorteia a raça da próxima carta, com chance proporcional
 * ao peso de cada uma na tabela. */
static TipoCachorro sortear_raca(void)
{
    int soma = 0;
    for (int t = 0; t < TOTAL_TIPOS_CACHORRO; t++) soma += pesos[t];

    int sorteio = rand() % soma;
    int tipo = 0;
    while (sorteio >= pesos[tipo]) {
        sorteio -= pesos[tipo];
        tipo++;
    }
    return (TipoCachorro)tipo;
}

/* retangulo_carta_esteira: área da carta i, na altura em que ela está agora. */
static Rectangle retangulo_carta_esteira(const Esteira *esteira, int i)
{
    return (Rectangle){ FAIXA.x + 5, esteira->cartas[i].y, FAIXA.width - 10, 36 };
}

/* esteira_iniciar: deixa a esteira vazia, sem carta selecionada e com o
 * timer zerado, para a 1ª carta chegar logo. */
void esteira_iniciar(Esteira *esteira)
{
    *esteira = (Esteira){ 0 };
    esteira->selecionada = -1;
}

/* esteira_atualizar: faz cada carta subir até a sua vaga (a vaga i fica
 * logo abaixo da i-1). A cada INTERVALO_ESTEIRA segundos chega uma carta
 * sorteada por baixo da tela; com a esteira cheia, essa carta é perdida. */
void esteira_atualizar(Esteira *esteira, float dt)
{
    esteira->movimento += VELOCIDADE_CARTA * dt;
    for (int i = 0; i < esteira->total; i++) {
        float vaga = ESTEIRA_TOPO + i * PASSO_VAGA;
        float y = esteira->cartas[i].y - VELOCIDADE_CARTA * dt;
        esteira->cartas[i].y = (y < vaga) ? vaga : y;
    }

    esteira->timer -= dt;
    if (esteira->timer > 0.0f) return;
    esteira->timer = INTERVALO_ESTEIRA;
    if (esteira->total < MAX_ESTEIRA) esteira->cartas[esteira->total++] = (CartaEsteira){ sortear_raca(), ALTURA_VIRTUAL };
}

/* esteira_carta_no_ponto: retorna o índice da carta que contém o ponto, ou
 * -1 se ele não estiver sobre nenhuma. */
int esteira_carta_no_ponto(const Esteira *esteira, Vector2 ponto)
{
    for (int i = 0; i < esteira->total; i++) {
        if (CheckCollisionPointRec(ponto, retangulo_carta_esteira(esteira, i))) return i;
    }
    return -1;
}

/* esteira_remover: tira a carta i do vetor, puxando as seguintes uma casa
 * para cima (elas sobem até a nova vaga na animação), e desmarca a seleção. */
void esteira_remover(Esteira *esteira, int i)
{
    for (int j = i; j < esteira->total - 1; j++) esteira->cartas[j] = esteira->cartas[j + 1];
    esteira->total--;
    esteira->selecionada = -1;
}

/* desenhar_dica_esteira: com o mouse sobre uma carta, mostra à direita dela
 * uma caixinha com o nome e o papel do cachorro. */
static void desenhar_dica_esteira(const Esteira *esteira, Vector2 mouse)
{
    int i = esteira_carta_no_ponto(esteira, mouse);
    if (i < 0) return;

    const InfoCachorro *info = cachorro_info(esteira->cartas[i].tipo);
    const char *texto = TextFormat("%s - %s", info->nome, info->papel);
    Rectangle caixa = { FAIXA.x + FAIXA.width + 4, esteira->cartas[i].y + 11, MeasureText(texto, FONTE_PEQUENA) + 8, 14 };
    DrawRectangleRec(caixa, Fade(BLACK, 0.85f));
    DrawRectangleLinesEx(caixa, 1, COR_PETISCO);
    ui_texto(texto, caixa.x + 4, caixa.y + 2, FONTE_PEQUENA, COR_TEXTO);
}

/* esteira_desenhar: a faixa escura com listras que sobem (dando a ideia de
 * movimento), as cartas com o retrato do cachorro (sem custo: são de graça), com borda verde na
 * selecionada e laranja sob o mouse, e a dica da carta sob o mouse. */
void esteira_desenhar(const Esteira *esteira, Vector2 mouse)
{
    float desvio = fmodf(esteira->movimento, ESPACO_LISTRAS);
    DrawRectangleRec(FAIXA, COR_ESTEIRA);
    for (float y = FAIXA.y + ESPACO_LISTRAS - desvio; y < FAIXA.y + FAIXA.height; y += ESPACO_LISTRAS) {
        DrawRectangle(FAIXA.x + 2, y, FAIXA.width - 4, 2, Fade(BLACK, 0.4f));
    }
    DrawRectangleLinesEx(FAIXA, 2, COR_BORDA);

    for (int i = 0; i < esteira->total; i++) {
        Rectangle r = retangulo_carta_esteira(esteira, i);
        bool mouseEmCima = CheckCollisionPointRec(mouse, r);

        DrawRectangleRec(r, (Color){ 196, 176, 140, 255 });
        cachorro_desenhar_retrato(esteira->cartas[i].tipo, (Vector2){ r.x + r.width / 2, r.y + r.height / 2 });
        DrawRectangleLinesEx(r, 2, (esteira->selecionada == i) ? COR_INFECTADO : (mouseEmCima ? COR_PETISCO : COR_BORDA));
    }
    desenhar_dica_esteira(esteira, mouse);
}
