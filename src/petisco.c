/* petisco.c - Lista encadeada de petiscos. */

#include "petisco.h"
#include "ui.h"

#include <math.h>
#include <stdlib.h>

#define RAIO_CLIQUE       13.0f
#define LARGURA_BISCOITO  15
#define ALTURA_BISCOITO   9
#define TAMANHO_ESTRELA   9

/* Figuras em pixel-art (matrizes de caracteres), nas cores do gerador_petiscos. */
static const char *BISCOITO[ALTURA_BISCOITO] = {
    ".##.........##.",
    "#aa#.......#aa#",
    "#aaa#######aaa#",
    ".#aaaaoaaaaaa#.",
    ".#aaaaaaaaoab#.",
    ".#bbbbbbbbbbb#.",
    "#aab#######aab#",
    "#bb#.......#bb#",
    ".##.........##.",
};
static const char *ESTRELA[TAMANHO_ESTRELA] = {
    "....#....",
    "...#y#...",
    "...#y#...",
    ".##ywy##.",
    "#yywwwyy#",
    ".##ywy##.",
    "...#y#...",
    "...#y#...",
    "....#....",
};
static const char SIMBOLOS[] = "#aboyw";
static const Color CORES[] = {
    {   0,   0,   0, 255 },   // # contorno
    { 238, 195, 154, 255 },   // a biscoito
    { 217, 160, 102, 255 },   // b sombra do biscoito
    { 143,  86,  59, 255 },   // o manchinhas
    { 251, 242,  54, 255 },   // y estrela
    { 255, 222, 191, 255 },   // w brilho da estrela
};

/* petisco_adicionar: aloca um petisco com malloc na posição dada (ele vai
 * cair até a altura alvoY) e o insere no INÍCIO da lista: o novo nó aponta
 * para a antiga cabeça e passa a ser a cabeça. Retorna false se faltar memória. */
bool petisco_adicionar(Petisco **lista, Vector2 posicao, float alvoY)
{
    Petisco *novo = malloc(sizeof(Petisco));
    if (novo == NULL) return false;

    novo->posicao = posicao;
    novo->alvoY = alvoY;
    novo->tempoNoChao = 0.0f;
    novo->proximo = *lista;
    *lista = novo;
    return true;
}

/* remover_no: tira da lista o nó apontado por 'ligacao' (que é a cabeça da
 * lista ou o campo 'proximo' do nó anterior), religando a lista ao nó
 * seguinte, e libera a memória do nó removido. */
static void remover_no(Petisco **ligacao)
{
    Petisco *removido = *ligacao;
    *ligacao = removido->proximo;
    free(removido);
}

/* petisco_atualizar_lista: percorre a lista fazendo cada petisco cair até a
 * sua altura alvo; depois de parado, conta o tempo no chão e remove os que
 * passaram de DURACAO_NO_CHAO. Percorre com um ponteiro para ponteiro, o que
 * permite remover um nó no meio da lista sem guardar o anterior. */
void petisco_atualizar_lista(Petisco **lista, float dt)
{
    Petisco **ligacao = lista;
    while (*ligacao != NULL) {
        Petisco *p = *ligacao;
        if (p->posicao.y < p->alvoY) p->posicao.y = fminf(p->posicao.y + VELOCIDADE_QUEDA * dt, p->alvoY);
        else                         p->tempoNoChao += dt;

        if (p->tempoNoChao > DURACAO_NO_CHAO) remover_no(ligacao);
        else                                  ligacao = &p->proximo;
    }
}

/* petisco_coletar: procura na lista um petisco sob o ponto clicado. Se achar,
 * remove o nó e retorna o valor dele; senão, retorna 0. */
int petisco_coletar(Petisco **lista, Vector2 ponto)
{
    for (Petisco **ligacao = lista; *ligacao != NULL; ligacao = &(*ligacao)->proximo) {
        if (CheckCollisionPointCircle(ponto, (*ligacao)->posicao, RAIO_CLIQUE)) {
            remover_no(ligacao);
            return VALOR_PETISCO;
        }
    }
    return 0;
}

/* petisco_desenhar_biscoito: desenha o biscoito em forma de osso centrado
 * em 'centro', com cada pixel ampliado 'escala' vezes. */
void petisco_desenhar_biscoito(Vector2 centro, int escala)
{
    Vector2 canto = { centro.x - LARGURA_BISCOITO * escala / 2, centro.y - ALTURA_BISCOITO * escala / 2 };
    ui_desenhar_pixel_art(BISCOITO, ALTURA_BISCOITO, canto, escala, SIMBOLOS, CORES);
}

/* petisco_desenhar_lista: desenha cada petisco como o biscoito (com sombra
 * depois que pousa) e, de vez em quando, a estrelinha brilhando na ponta,
 * como no sprite do gerador. Nos últimos 2,5 segundos antes de sumir, ele
 * pisca. */
void petisco_desenhar_lista(const Petisco *lista, float tempo)
{
    for (const Petisco *p = lista; p != NULL; p = p->proximo) {
        if (p->tempoNoChao > DURACAO_NO_CHAO - 2.5f && fmodf(tempo, 0.25f) < 0.12f) continue;

        if (p->posicao.y >= p->alvoY) DrawEllipse(p->posicao.x, p->posicao.y + 6, 8, 2, Fade(BLACK, 0.3f));
        petisco_desenhar_biscoito(p->posicao, 1);
        if (fmodf(tempo + p->alvoY * 0.01f, 1.2f) < 0.3f) {
            ui_desenhar_pixel_art(ESTRELA, TAMANHO_ESTRELA, (Vector2){ p->posicao.x + 3, p->posicao.y - 12 }, 1, SIMBOLOS, CORES);
        }
    }
}

/* petisco_liberar_lista: remove a cabeça da lista até ela ficar vazia (NULL),
 * liberando todos os nós. */
void petisco_liberar_lista(Petisco **lista)
{
    while (*lista != NULL) remover_no(lista);
}
