/* petisco.c - Lista encadeada de petiscos. */

#include "petisco.h"
#include "ui.h"

#include <math.h>
#include <stdlib.h>

#define RAIO_CLIQUE 13.0f

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

/* petisco_desenhar_lista: desenha cada petisco como um osso com brilho
 * laranja. Nos últimos 2,5 segundos antes de sumir, ele pisca. */
void petisco_desenhar_lista(const Petisco *lista, float tempo)
{
    for (const Petisco *p = lista; p != NULL; p = p->proximo) {
        if (p->tempoNoChao > DURACAO_NO_CHAO - 2.5f && fmodf(tempo, 0.25f) < 0.12f) continue;
        DrawCircleV(p->posicao, 11, Fade(COR_PETISCO, 0.3f));
        ui_desenhar_osso(p->posicao, 1.0f, COR_OSSO);
    }
}

/* petisco_liberar_lista: remove a cabeça da lista até ela ficar vazia (NULL),
 * liberando todos os nós. */
void petisco_liberar_lista(Petisco **lista)
{
    while (*lista != NULL) remover_no(lista);
}
