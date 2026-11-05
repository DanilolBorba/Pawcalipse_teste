/* projetil.c - Lista encadeada das babas em voo e o desenho de cada estilo. */

#include "projetil.h"
#include "ui.h"

#include <stdlib.h>
#include <string.h>

/* Cada estilo de baba em pixel-art (matrizes de caracteres), voando para a direita. */
static const char *GOTA[] = {
    "......###..",
    "....##aab#.",
    "..##aaaabb#",
    "##aaaaaaaa#",
    "..##caaaaa#",
    "....##cca#.",
    "......###..",
};
static const char *BOLHAS[] = {
    ".....###.",
    "....#abb#",
    "###.#aaa#",
    "#ab#.###.",
    "#aa#..##.",
    ".###.#ab#",
    "......##.",
};
static const char *CRISTAL[] = {
    "...#...",
    "..#b#..",
    ".#bac#.",
    "#baacc#",
    ".#acc#.",
    "..#c#..",
    "...#...",
};
static const char *JATO[] = {
    "........###.",
    "a.aa.###aab#",
    ".a..##ccaaa#",
    "........###.",
};
static const char SIMBOLOS[] = "#abc";
static const Color CORES_BABA[] = { {  70, 110, 130, 255 }, { 205, 235, 245, 255 }, { 255, 255, 255, 255 }, { 150, 195, 215, 255 } };
static const Color CORES_GELO[] = { {  40,  90, 140, 255 }, { 170, 220, 255, 255 }, { 255, 255, 255, 255 }, { 110, 170, 230, 255 } };

typedef struct DesenhoBaba {
    const char **figura;
    int          linhas;
    const Color *cores;     // cores de '#' (contorno), 'a', 'b' (brilho) e 'c' (sombra)
} DesenhoBaba;

static const DesenhoBaba desenhos[TOTAL_ESTILOS_BABA] = {
    [BABA_GOTA]   = { GOTA,    7, CORES_BABA },
    [BABA_BOLHAS] = { BOLHAS,  7, CORES_BABA },
    [BABA_GELADA] = { CRISTAL, 7, CORES_GELO },
    [BABA_JATO]   = { JATO,    4, CORES_BABA },
};

/* projetil_adicionar: aloca uma baba com malloc na posição de disparo e a
 * insere no INÍCIO da lista. Retorna false se faltar memória. */
bool projetil_adicionar(Projetil **lista, Vector2 posicao, int linha, int dano, EstiloBaba estilo)
{
    Projetil *novo = malloc(sizeof(Projetil));
    if (novo == NULL) return false;

    novo->posicao = posicao;
    novo->linha = linha;
    novo->dano = dano;
    novo->estilo = estilo;
    novo->proximo = *lista;
    *lista = novo;
    return true;
}

/* remover_no: tira da lista o nó apontado por 'ligacao' (a cabeça ou o campo
 * 'proximo' do nó anterior), religa a lista ao nó seguinte e libera o nó. */
static void remover_no(Projetil **ligacao)
{
    Projetil *removido = *ligacao;
    *ligacao = removido->proximo;
    free(removido);
}

/* projetil_atualizar_lista: move cada baba para a direita; se ela tocar
 * um gato da sua linha, causa o dano nele (e, se for gelada, o deixa lento).
 * Remove da lista as que acertaram e as que passaram de fimX (a borda da
 * tela ou, na fase do chefão, a frente do Rei). Retorna a soma do dano das
 * babas que chegaram a fimX sem acertar nenhum gato. */
int projetil_atualizar_lista(Projetil **lista, Gato *gatos, float fimX, float dt)
{
    int danoNoFim = 0;
    Projetil **ligacao = lista;
    while (*ligacao != NULL) {
        Projetil *p = *ligacao;
        p->posicao.x += VELOCIDADE_PROJETIL * dt;

        Gato *alvo = gato_atingido(gatos, p->linha, p->posicao.x);
        if (alvo != NULL) gato_ferir(alvo, p->dano);
        if (alvo != NULL && p->estilo == BABA_GELADA) gato_gelar(alvo);
        if (alvo == NULL && p->posicao.x > fimX) danoNoFim += p->dano;

        if (alvo != NULL || p->posicao.x > fimX) remover_no(ligacao);
        else                                     ligacao = &p->proximo;
    }
    return danoNoFim;
}

/* projetil_desenhar_lista: desenha cada baba com a pixel-art do seu estilo
 * (tabela de desenhos), centrada na posição dela. */
void projetil_desenhar_lista(const Projetil *lista)
{
    for (const Projetil *p = lista; p != NULL; p = p->proximo) {
        const DesenhoBaba *d = &desenhos[p->estilo];
        Vector2 canto = { p->posicao.x - (int)strlen(d->figura[0]) / 2, p->posicao.y - d->linhas / 2 };
        ui_desenhar_pixel_art(d->figura, d->linhas, canto, 1, SIMBOLOS, d->cores);
    }
}

/* projetil_liberar_lista: remove a cabeça da lista até ela ficar vazia (NULL),
 * liberando todos os nós. */
void projetil_liberar_lista(Projetil **lista)
{
    while (*lista != NULL) remover_no(lista);
}
