#ifndef ENEMY_H
#define ENEMY_H

/* enemy.h - Os gatos zumbis. Os gatos em campo ficam numa LISTA ENCADEADA:
 * cada um é criado com malloc quando entra pela rua e liberado com free
 * quando morre. */

#include "raylib.h"
#include "map.h"
#include "sprite.h"
#include <stdbool.h>

#define RAIO_GATO           22     // metade da largura do corpo (área onde ele é atingido)
#define VIDA_SEM_ACESSORIO  200    // abaixo disso o gato perde a caixa ou a lata
#define INTERVALO_MORDIDA   1.0f   // segundos entre duas mordidas
#define DURACAO_LENTIDAO    4.0f   // segundos lento depois de uma bolinha gelada

typedef enum TipoGato {
    GATO_PADRAO,       // o mais fraco
    GATO_CAIXA,        // protegido por uma caixa de leite: mais resistente
    GATO_LATA,         // protegido por uma lata: o mais forte dos três
    GATO_SIAMES,       // rápido
    GATO_PERSA,        // pelo fofo: imune ao gelo
    GATO_TELHADO,      // pula o primeiro cachorro (menos o Bulldog)
    GATO_MAINE_COON,   // tanque: enorme e com mordida forte
    TOTAL_TIPOS_GATO
} TipoGato;

typedef struct InfoGato {   // dados fixos de cada tipo de gato (provisórios)
    const char *nome;
    int      vidaMaxima;
    float    velocidade;    // pixels por segundo
    int      danoMordida;
    bool     imuneGelo;
    bool     pula;          // pula o primeiro cachorro que encontrar
    float    escala;        // tamanho do desenho (1 = 64x64)
    IdSprite sprite;        // animação de andar (com o acessório, se tiver)
    Color    tinta;         // cor multiplicada no sprite (as variações do gato padrão)
} InfoGato;

typedef struct Gato {
    TipoGato tipo;
    int   linha;            // linha do mapa por onde ele anda
    float x;                // posição horizontal do centro (o y vem da linha)
    int   vida;
    float timerMordida;     // segundos até a próxima mordida
    float timerDano;        // segundos com a cor de dano
    float timerLento;       // segundos restantes de lentidão (gelo)
    float animacao;         // segundos da animação de andar já tocados
    bool  mordendo;         // parado mordendo um cachorro
    bool  pulou;            // o Gato de Telhado já usou o pulo
    struct Gato *proximo;   // próximo nó da lista (NULL = fim)
} Gato;

const InfoGato *gato_info(TipoGato tipo);                          // dados fixos de um tipo
bool  gato_adicionar(Gato **lista, TipoGato tipo, int linha);      // novo gato na rua, no início da lista (false se faltar memória)
bool  gato_atualizar_lista(Gato **lista, Mapa *mapa, float dt);    // anda/morde e remove os mortos; true se algum invadiu a casa
bool  gato_na_mira(const Gato *lista, int linha, float x);         // true se há um gato visível na linha, à direita de x
Gato *gato_atingido(Gato *lista, int linha, float x);              // gato vivo da linha que ocupa o ponto x (NULL se nenhum)
int   gato_ferir_area(Gato *lista, int linha, float xMin, float xMax, int dano);  // fere os gatos da linha entre xMin e xMax; retorna quantos
void  gato_ferir(Gato *gato, int dano);                            // tira vida e muda a cor por um instante
void  gato_gelar(Gato *gato);                                      // deixa o gato lento (se não for imune)
void  gato_desenhar_lista(const Gato *lista);                      // desenha todos os gatos
void  gato_liberar_lista(Gato **lista);                            // libera todos os nós e deixa a lista vazia

#endif
