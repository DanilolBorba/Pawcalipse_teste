#ifndef ENEMY_H
#define ENEMY_H

/* enemy.h - Os gatos zumbis. Os gatos em campo ficam numa LISTA ENCADEADA:
 * cada um é criado com malloc quando entra pela rua e liberado com free
 * quando morre. */

#include "raylib.h"
#include "map.h"
#include <stdbool.h>

#define RAIO_GATO           16     // tamanho do gato (e da área onde ele é atingido)
#define VIDA_SEM_ACESSORIO  200    // abaixo disso o gato perde o cone ou a panela
#define INTERVALO_MORDIDA   1.0f   // segundos entre duas mordidas

typedef enum TipoGato {
    GATO_COMUM,
    GATO_CONE,     // protegido por um cone de veterinário
    GATO_PANELA,   // protegido por uma panela na cabeça
    TOTAL_TIPOS_GATO
} TipoGato;

typedef struct InfoGato {   // dados fixos de cada tipo de gato (provisórios)
    const char *nome;
    int   vidaMaxima;
    float velocidade;       // pixels por segundo
    int   danoMordida;
} InfoGato;

typedef struct Gato {
    TipoGato tipo;
    int   linha;            // linha do mapa por onde ele anda
    float x;                // posição horizontal do centro (o y vem da linha)
    int   vida;
    float timerMordida;     // segundos até a próxima mordida
    float timerDano;        // segundos piscando em branco depois de levar dano
    struct Gato *proximo;   // próximo nó da lista (NULL = fim)
} Gato;

const InfoGato *gato_info(TipoGato tipo);                          // dados fixos de um tipo
bool  gato_adicionar(Gato **lista, TipoGato tipo, int linha);      // novo gato na rua, no início da lista (false se faltar memória)
bool  gato_atualizar_lista(Gato **lista, Mapa *mapa, float dt);    // anda/morde e remove os mortos; true se algum invadiu a casa
bool  gato_na_mira(const Gato *lista, int linha, float x);         // true se há um gato visível na linha, à direita de x
Gato *gato_atingido(Gato *lista, int linha, float x);              // gato vivo da linha que ocupa o ponto x (NULL se nenhum)
void  gato_ferir(Gato *gato, int dano);                            // tira vida e faz o gato piscar
void  gato_desenhar_lista(const Gato *lista);                      // desenha todos os gatos
void  gato_liberar_lista(Gato **lista);                            // libera todos os nós e deixa a lista vazia

#endif
