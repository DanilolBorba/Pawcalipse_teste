#ifndef PETISCO_H
#define PETISCO_H

/* petisco.h - Petiscos (ossos) que caem do céu ou saem do Pote de Ração.
 * Ficam numa LISTA ENCADEADA: cada um é criado com malloc quando aparece e
 * liberado com free quando é coletado ou some do chão. */

#include "raylib.h"
#include <stdbool.h>

#define VALOR_PETISCO     25      // petiscos ganhos por osso coletado
#define VELOCIDADE_QUEDA  36.0f   // pixels por segundo
#define DURACAO_NO_CHAO   10.0f   // segundos parado até sumir

typedef struct Petisco {
    Vector2 posicao;
    float   alvoY;             // altura onde ele para de cair
    float   tempoNoChao;       // segundos desde que parou
    struct Petisco *proximo;   // próximo nó da lista (NULL = fim)
} Petisco;

bool petisco_adicionar(Petisco **lista, Vector2 posicao, float alvoY);  // novo petisco no início da lista (false se faltar memória)
void petisco_atualizar_lista(Petisco **lista, float dt);                // faz cair e remove os que passaram do tempo
int  petisco_coletar(Petisco **lista, Vector2 ponto);                   // remove o petisco clicado; retorna o valor (0 se nenhum)
void petisco_desenhar_lista(const Petisco *lista, float tempo);         // desenha todos os petiscos
void petisco_liberar_lista(Petisco **lista);                            // libera todos os nós e deixa a lista vazia

#endif
