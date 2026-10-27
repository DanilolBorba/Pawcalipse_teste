/* projetil.c - Lista encadeada das bolinhas de tênis em voo. */

#include "projetil.h"
#include "config.h"

#include <stdlib.h>

/* projetil_adicionar: aloca uma bolinha com malloc na posição de disparo e a
 * insere no INÍCIO da lista. Retorna false se faltar memória. */
bool projetil_adicionar(Projetil **lista, Vector2 posicao, int linha, int dano)
{
    Projetil *novo = malloc(sizeof(Projetil));
    if (novo == NULL) return false;

    novo->posicao = posicao;
    novo->linha = linha;
    novo->dano = dano;
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

/* projetil_atualizar_lista: move cada bolinha para a direita; se ela tocar
 * um gato da sua linha, causa o dano nele. Remove da lista as que acertaram
 * e as que saíram da tela. */
void projetil_atualizar_lista(Projetil **lista, Gato *gatos, float dt)
{
    Projetil **ligacao = lista;
    while (*ligacao != NULL) {
        Projetil *p = *ligacao;
        p->posicao.x += VELOCIDADE_PROJETIL * dt;

        Gato *alvo = gato_atingido(gatos, p->linha, p->posicao.x);
        if (alvo != NULL) gato_ferir(alvo, p->dano);

        if (alvo != NULL || p->posicao.x > LARGURA_VIRTUAL) remover_no(ligacao);
        else                                                ligacao = &p->proximo;
    }
}

/* projetil_desenhar_lista: desenha cada bolinha de tênis (amarelo-esverdeada
 * com contorno escuro). */
void projetil_desenhar_lista(const Projetil *lista)
{
    for (const Projetil *p = lista; p != NULL; p = p->proximo) {
        DrawCircleV(p->posicao, 4, (Color){ 210, 232, 60, 255 });
        DrawCircleLinesV(p->posicao, 4, (Color){ 90, 104, 20, 255 });
    }
}

/* projetil_liberar_lista: remove a cabeça da lista até ela ficar vazia (NULL),
 * liberando todos os nós. */
void projetil_liberar_lista(Projetil **lista)
{
    while (*lista != NULL) remover_no(lista);
}
