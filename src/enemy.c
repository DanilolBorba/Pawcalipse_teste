/* enemy.c - Tabela dos gatos e a lista encadeada dos gatos em campo. */

#include "enemy.h"
#include "config.h"
#include "ui.h"

#include <math.h>
#include <stdlib.h>

#define ALCANCE_MORDIDA  10.0f   // distância do centro do gato até o ponto que ele morde
#define TEMPO_PISCANDO   0.12f

static const InfoGato infos[TOTAL_TIPOS_GATO] = {
    /*                nome              vida  velocidade  mordida */
    [GATO_COMUM]  = { "Gato Zumbi",      200,  12.0f,      100 },
    [GATO_CONE]   = { "Gato de Cone",    560,  12.0f,      100 },
    [GATO_PANELA] = { "Gato de Panela", 1300,  12.0f,      100 },
};

/* gato_info: retorna um ponteiro para os dados fixos do tipo na tabela. */
const InfoGato *gato_info(TipoGato tipo)
{
    return &infos[tipo];
}

/* gato_adicionar: aloca um gato com malloc, com a vida cheia do tipo, logo
 * depois da borda direita da tela (na rua), e o insere no INÍCIO da lista.
 * Retorna false se faltar memória. */
bool gato_adicionar(Gato **lista, TipoGato tipo, int linha)
{
    Gato *novo = malloc(sizeof(Gato));
    if (novo == NULL) return false;

    novo->tipo = tipo;
    novo->linha = linha;
    novo->x = LARGURA_VIRTUAL + RAIO_GATO;
    novo->vida = infos[tipo].vidaMaxima;
    novo->timerMordida = 0.0f;
    novo->timerDano = 0.0f;
    novo->proximo = *lista;
    *lista = novo;
    return true;
}

/* remover_no: tira da lista o nó apontado por 'ligacao' (a cabeça ou o campo
 * 'proximo' do nó anterior), religa a lista ao nó seguinte e libera o nó. */
static void remover_no(Gato **ligacao)
{
    Gato *removido = *ligacao;
    *ligacao = removido->proximo;
    free(removido);
}

/* cachorro_a_frente: retorna o cachorro da célula onde fica a "boca" do gato
 * (um pouco à esquerda do centro), ou NULL se o caminho estiver livre. */
static Cachorro *cachorro_a_frente(const Gato *gato, Mapa *mapa)
{
    int linha, coluna;
    Vector2 boca = { gato->x - ALCANCE_MORDIDA, mapa_centro_celula(gato->linha, 0).y };
    if (!mapa_celula_no_ponto(boca, &linha, &coluna)) return NULL;
    return mapa->grade[linha][coluna];
}

/* mover_ou_morder: com o caminho livre, o gato anda para a esquerda. Com um
 * cachorro na frente, ele para e morde a cada INTERVALO_MORDIDA segundos; o
 * cachorro que fica sem vida é removido do mapa (e liberado). */
static void mover_ou_morder(Gato *gato, Mapa *mapa, float dt)
{
    const InfoGato *info = &infos[gato->tipo];
    Cachorro *alvo = cachorro_a_frente(gato, mapa);
    if (alvo == NULL) {
        gato->x -= info->velocidade * dt;
        return;
    }

    gato->timerMordida -= dt;
    if (gato->timerMordida > 0.0f) return;

    gato->timerMordida = INTERVALO_MORDIDA;
    alvo->vida -= info->danoMordida;
    if (alvo->vida <= 0) mapa_remover(mapa, alvo->linha, alvo->coluna);
}

/* gato_atualizar_lista: percorre a lista com um ponteiro para ponteiro:
 * remove os gatos sem vida e faz os outros andarem ou morderem. Retorna true
 * se algum gato passou da coluna da esquerda e entrou na casa. */
bool gato_atualizar_lista(Gato **lista, Mapa *mapa, float dt)
{
    bool invadiu = false;
    Gato **ligacao = lista;
    while (*ligacao != NULL) {
        Gato *gato = *ligacao;
        if (gato->vida <= 0) {
            remover_no(ligacao);
            continue;
        }

        if (gato->timerDano > 0.0f) gato->timerDano -= dt;
        mover_ou_morder(gato, mapa, dt);
        if (gato->x < MAPA_X) invadiu = true;
        ligacao = &gato->proximo;
    }
    return invadiu;
}

/* gato_na_mira: retorna true se existe um gato vivo na linha, à direita do
 * ponto x e já dentro da tela (usado pelos atiradores para decidir se atiram). */
bool gato_na_mira(const Gato *lista, int linha, float x)
{
    for (const Gato *g = lista; g != NULL; g = g->proximo) {
        if (g->linha == linha && g->vida > 0 && g->x > x && g->x < LARGURA_VIRTUAL) return true;
    }
    return false;
}

/* gato_atingido: procura um gato vivo na linha cujo corpo cubra o ponto x.
 * Retorna o ponteiro para ele, ou NULL se não houver nenhum. */
Gato *gato_atingido(Gato *lista, int linha, float x)
{
    for (Gato *g = lista; g != NULL; g = g->proximo) {
        if (g->linha == linha && g->vida > 0 && fabsf(g->x - x) < RAIO_GATO) return g;
    }
    return NULL;
}

/* gato_ferir: tira 'dano' da vida do gato e o faz piscar em branco. O gato
 * sem vida sai da lista na próxima atualização. */
void gato_ferir(Gato *gato, int dano)
{
    gato->vida -= dano;
    gato->timerDano = TEMPO_PISCANDO;
}

/* desenhar_acessorio: desenha a proteção do gato em volta da cabeça 'cabeca':
 * o cone de veterinário (funil translúcido aberto para a frente) ou a panela
 * (com o cabo para trás). O gato comum não tem acessório. */
static void desenhar_acessorio(TipoGato tipo, Vector2 cabeca)
{
    if (tipo == GATO_CONE) {
        Vector2 ponta = { cabeca.x + 6, cabeca.y };
        Vector2 cima = { cabeca.x - 13, cabeca.y - 15 }, baixo = { cabeca.x - 13, cabeca.y + 15 };
        DrawTriangle(ponta, cima, baixo, Fade(RAYWHITE, 0.55f));
        DrawTriangleLines(ponta, cima, baixo, (Color){ 150, 170, 190, 255 });
    } else if (tipo == GATO_PANELA) {
        DrawRectangle(cabeca.x - 11, cabeca.y - 16, 22, 9, (Color){ 150, 150, 160, 255 });
        DrawRectangle(cabeca.x - 13, cabeca.y - 8, 26, 2, (Color){ 100, 100, 110, 255 });
        DrawRectangle(cabeca.x + 11, cabeca.y - 14, 8, 3, (Color){ 60, 50, 50, 255 });
    }
}

/* desenhar_gato: desenha o gato com formas geométricas (provisório até os
 * sprites), virado para a esquerda: sombra, rabo, corpo, cabeça, orelhas,
 * olhos vermelhos e o acessório, se a vida ainda estiver acima de
 * VIDA_SEM_ACESSORIO. Ele balança conforme anda e pisca em branco ao levar
 * dano. Se estiver ferido, mostra a barra de vida. */
static void desenhar_gato(const Gato *gato)
{
    const InfoGato *info = &infos[gato->tipo];
    Vector2 c = { gato->x, mapa_centro_celula(gato->linha, 0).y + 4 + sinf(gato->x * 0.4f) * 1.5f };
    Vector2 cabeca = { c.x - 8, c.y - 6 };
    Color pelo = (gato->timerDano > 0.0f) ? WHITE : (Color){ 118, 146, 98, 255 };

    DrawEllipse(c.x, c.y + 13, 15, 4, Fade(BLACK, 0.3f));
    DrawLineEx((Vector2){ c.x + 10, c.y + 2 }, (Vector2){ c.x + 16, c.y - 12 }, 3, pelo);
    DrawEllipse(c.x + 3, c.y + 4, 13, 8, pelo);
    DrawPoly((Vector2){ cabeca.x - 5, cabeca.y - 8 }, 3, 5, -90, pelo);
    DrawPoly((Vector2){ cabeca.x + 5, cabeca.y - 8 }, 3, 5, -90, pelo);
    DrawCircleV(cabeca, 9, pelo);
    DrawCircleV((Vector2){ cabeca.x - 5, cabeca.y - 1 }, 2, COR_SANGUE);
    DrawCircleV((Vector2){ cabeca.x + 2, cabeca.y - 1 }, 2, COR_SANGUE);

    if (gato->vida > VIDA_SEM_ACESSORIO) desenhar_acessorio(gato->tipo, cabeca);
    if (gato->vida < info->vidaMaxima) ui_desenhar_barra_vida((Vector2){ c.x, c.y - 26 }, 26, (float)gato->vida / info->vidaMaxima);
}

/* gato_desenhar_lista: percorre a lista desenhando cada gato. */
void gato_desenhar_lista(const Gato *lista)
{
    for (const Gato *g = lista; g != NULL; g = g->proximo) desenhar_gato(g);
}

/* gato_liberar_lista: remove a cabeça da lista até ela ficar vazia (NULL),
 * liberando todos os nós. */
void gato_liberar_lista(Gato **lista)
{
    while (*lista != NULL) remover_no(lista);
}
