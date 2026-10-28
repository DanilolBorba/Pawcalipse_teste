/* enemy.c - Tabela dos gatos e a lista encadeada dos gatos em campo. */

#include "enemy.h"
#include "config.h"
#include "ui.h"

#include <math.h>
#include <stdlib.h>

#define ALCANCE_MORDIDA     14.0f   // distância do centro do gato até o ponto que ele morde
#define TEMPO_COR_DANO      0.12f
#define VELOCIDADE_DA_ARTE  3.2f    // px/s que a animação de andar "anda" (ver o LEIAME dos sprites)

static const InfoGato infos[TOTAL_TIPOS_GATO] = {
    /*                     nome              vida  velocidade mordida  imuneGelo  pula  escala  sprite              tinta */
    [GATO_PADRAO]     = { "Gato Zumbi",       200,   8.0f,     100,    false,     false, 1.00f, SPRITE_GATO_PADRAO, { 255, 255, 255, 255 } },
    [GATO_CAIXA]      = { "Gato da Caixa",    560,   8.0f,     100,    false,     false, 1.00f, SPRITE_GATO_CAIXA,  { 255, 255, 255, 255 } },
    [GATO_LATA]       = { "Gato da Lata",    1300,   8.0f,     100,    false,     false, 1.00f, SPRITE_GATO_LATA,   { 255, 255, 255, 255 } },
    [GATO_SIAMES]     = { "Siamês",           180,  16.0f,     100,    false,     false, 1.00f, SPRITE_GATO_PADRAO, { 245, 220, 175, 255 } },
    [GATO_PERSA]      = { "Persa",            450,   7.0f,     100,    true,      false, 1.15f, SPRITE_GATO_PADRAO, { 235, 235, 250, 255 } },
    [GATO_TELHADO]    = { "Gato de Telhado",  250,   9.0f,     100,    false,     true,  1.00f, SPRITE_GATO_PADRAO, { 165, 165, 180, 255 } },
    [GATO_MAINE_COON] = { "Maine Coon",      3000,   5.0f,     300,    false,     false, 1.50f, SPRITE_GATO_PADRAO, { 205, 145,  95, 255 } },
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

    *novo = (Gato){ 0 };
    novo->tipo = tipo;
    novo->linha = linha;
    novo->x = LARGURA_VIRTUAL + RAIO_GATO * infos[tipo].escala;
    novo->vida = infos[tipo].vidaMaxima;
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
    Vector2 boca = { gato->x - ALCANCE_MORDIDA * infos[gato->tipo].escala, mapa_centro_celula(gato->linha, 0).y };
    if (!mapa_celula_no_ponto(boca, &linha, &coluna)) return NULL;
    return mapa->grade[linha][coluna];
}

/* mover_ou_morder: com o caminho livre, o gato anda para a esquerda e avança
 * a animação no mesmo ritmo (para as patas não deslizarem). Com um cachorro
 * na frente, o Gato de Telhado tenta pular uma vez (o Bulldog é alto demais);
 * os outros param e mordem a cada INTERVALO_MORDIDA segundos, e o cachorro
 * que fica sem vida é removido do mapa. Lento pelo gelo, tudo vai à metade. */
static void mover_ou_morder(Gato *gato, Mapa *mapa, float dt)
{
    const InfoGato *info = &infos[gato->tipo];
    float passo = (gato->timerLento > 0.0f) ? dt * 0.5f : dt;
    Cachorro *alvo = cachorro_a_frente(gato, mapa);

    gato->mordendo = (alvo != NULL);
    if (alvo == NULL) {
        gato->x -= info->velocidade * passo;
        gato->animacao += info->velocidade * passo / VELOCIDADE_DA_ARTE;
        return;
    }
    if (info->pula && !gato->pulou) {
        gato->pulou = true;
        if (alvo->tipo != TIPO_BULLDOG) gato->x -= TAMANHO_CELULA;
        return;
    }

    gato->timerMordida -= passo;
    if (gato->timerMordida > 0.0f) return;

    gato->timerMordida = INTERVALO_MORDIDA;
    alvo->vida -= info->danoMordida;
    if (alvo->vida <= 0) mapa_remover(mapa, alvo->linha, alvo->coluna);
}

/* gato_atualizar_lista: percorre a lista com um ponteiro para ponteiro:
 * remove os gatos sem vida, conta os tempos de dano e de lentidão e faz os
 * outros andarem ou morderem. Retorna true se algum gato passou da coluna da
 * esquerda e entrou na casa. */
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
        if (gato->timerLento > 0.0f) gato->timerLento -= dt;
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
        if (g->linha == linha && g->vida > 0 && fabsf(g->x - x) < RAIO_GATO * infos[g->tipo].escala) return g;
    }
    return NULL;
}

/* gato_empurrar_area: faz todos os gatos vivos da linha cujo centro esteja
 * entre xMin e xMax recuarem 'distancia' pixels para a direita (o susto do
 * grito do Shiba). Retorna quantos recuaram (0 se nenhum). */
int gato_empurrar_area(Gato *lista, int linha, float xMin, float xMax, float distancia)
{
    int empurrados = 0;
    for (Gato *g = lista; g != NULL; g = g->proximo) {
        if (g->linha != linha || g->vida <= 0 || g->x < xMin || g->x > xMax) continue;
        g->x += distancia;
        empurrados++;
    }
    return empurrados;
}

/* gato_ferir_area: fere com 'dano' todos os gatos vivos da linha cujo centro
 * esteja entre xMin e xMax (mordidas e explosões). Retorna quantos foram
 * atingidos (0 se nenhum). */
int gato_ferir_area(Gato *lista, int linha, float xMin, float xMax, int dano)
{
    int atingidos = 0;
    for (Gato *g = lista; g != NULL; g = g->proximo) {
        if (g->linha != linha || g->vida <= 0 || g->x < xMin || g->x > xMax) continue;
        gato_ferir(g, dano);
        atingidos++;
    }
    return atingidos;
}

/* gato_ferir: tira 'dano' da vida do gato e o deixa avermelhado por um
 * instante. O gato sem vida sai da lista na próxima atualização. */
void gato_ferir(Gato *gato, int dano)
{
    gato->vida -= dano;
    gato->timerDano = TEMPO_COR_DANO;
}

/* gato_gelar: deixa o gato lento por DURACAO_LENTIDAO segundos, a não ser
 * que ele seja imune ao gelo (Persa). */
void gato_gelar(Gato *gato)
{
    if (!infos[gato->tipo].imuneGelo) gato->timerLento = DURACAO_LENTIDAO;
}

/* tinta_do_gato: cor multiplicada no desenho: avermelhada logo depois de
 * levar dano, azulada enquanto estiver lento e, senão, a tinta do tipo. */
static Color tinta_do_gato(const Gato *gato)
{
    if (gato->timerDano > 0.0f) return (Color){ 255, 110, 110, 255 };
    if (gato->timerLento > 0.0f) return ColorTint(infos[gato->tipo].tinta, (Color){ 150, 190, 255, 255 });
    return infos[gato->tipo].tinta;
}

/* desenhar_gato_formas: desenho provisório, usado se o PNG não existir, com
 * os pés em 'pes': rabo, corpo, orelhas, cabeça, olhos vermelhos e, se ainda
 * tiver, a caixa (branca) ou a lata (cinza) na cabeça. */
static void desenhar_gato_formas(const Gato *gato, Vector2 pes, Color tinta)
{
    Color pelo = ColorTint((Color){ 118, 146, 98, 255 }, tinta);
    Vector2 cabeca = { pes.x - 10, pes.y - 26 };

    DrawLineEx((Vector2){ pes.x + 12, pes.y - 12 }, (Vector2){ pes.x + 18, pes.y - 26 }, 3, pelo);
    DrawEllipse(pes.x + 3, pes.y - 10, 15, 9, pelo);
    DrawPoly((Vector2){ cabeca.x - 5, cabeca.y - 8 }, 3, 5, -90, pelo);
    DrawPoly((Vector2){ cabeca.x + 5, cabeca.y - 8 }, 3, 5, -90, pelo);
    DrawCircleV(cabeca, 9, pelo);
    DrawCircleV((Vector2){ cabeca.x - 5, cabeca.y - 1 }, 2, COR_SANGUE);
    DrawCircleV((Vector2){ cabeca.x + 2, cabeca.y - 1 }, 2, COR_SANGUE);

    if (gato->vida <= VIDA_SEM_ACESSORIO) return;
    if (gato->tipo == GATO_CAIXA) DrawRectangle(cabeca.x - 10, cabeca.y - 20, 20, 16, RAYWHITE);
    if (gato->tipo == GATO_LATA)  DrawRectangle(cabeca.x - 10, cabeca.y - 16, 20, 9, GRAY);
}

/* desenhar_gato: desenha o gato com os pés perto da base da sua linha. Com o
 * PNG carregado, usa o quadro da animação de andar (o sprite da caixa ou da
 * lata só enquanto ele tiver o acessório); mordendo, fica no 1º quadro e dá
 * um tranco para a frente a cada mordida. Sem o PNG, usa as formas. Ferido,
 * mostra a barra de vida acima da cabeça. */
static void desenhar_gato(const Gato *gato)
{
    const InfoGato *info = &infos[gato->tipo];
    IdSprite sprite = (gato->vida > VIDA_SEM_ACESSORIO) ? info->sprite : SPRITE_GATO_PADRAO;
    Vector2 pes = { gato->x, mapa_centro_celula(gato->linha, 0).y + 26 };
    int quadro = gato->mordendo ? 0 : sprite_quadro_no_tempo(sprite, gato->animacao);

    if (gato->mordendo && gato->timerMordida > INTERVALO_MORDIDA - 0.15f) pes.x -= 3;
    DrawEllipse(pes.x, pes.y, 16 * info->escala, 4 * info->escala, Fade(BLACK, 0.3f));
    if (sprite_existe(sprite)) sprite_desenhar(sprite, quadro, pes, info->escala, tinta_do_gato(gato));
    else                       desenhar_gato_formas(gato, pes, tinta_do_gato(gato));

    if (gato->vida < info->vidaMaxima) {
        ui_desenhar_barra_vida((Vector2){ gato->x, pes.y - 54 * info->escala }, 26, (float)gato->vida / info->vidaMaxima);
    }
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
