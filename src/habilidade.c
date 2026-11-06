/* habilidade.c - As habilidades das raças e a tabela de ponteiros de função
 * que liga cada raça à sua. */

#include "habilidade.h"
#include "som.h"

#include <stdlib.h>

#define ALCANCE_PASTOR  (1.5f * TAMANHO_CELULA)   // até onde a mordida do Pastor Alemão alcança
#define RAIO_3X3        (1.5f * TAMANHO_CELULA)   // metade da largura da área 3x3
#define ALCANCE_GRITO   (2.5f * TAMANHO_CELULA)   // até onde o grito do Shiba Inu assusta
#define RECUO_GRITO     (0.5f * TAMANHO_CELULA)   // quanto os gatos assustados recuam

static const Color COR_EXPLOSAO = { 255, 150,  40, 255 };
static const Color COR_CURA     = { 130, 230, 120, 255 };
static const Color COR_TERRA    = { 170, 130,  90, 255 };
static const Color COR_GRITO    = { 255, 225, 120, 255 };

/* centro_do: centro da célula onde o cachorro está. */
static Vector2 centro_do(const Cachorro *cachorro)
{
    return mapa_centro_celula(cachorro->linha, cachorro->coluna);
}

/* tem_alvo: true se há um gato na linha à frente do ponto x ou se é a fase
 * do chefão (aí sempre vale atirar: as babas que passam acertam o Rei). */
static bool tem_alvo(const Partida *partida, int linha, float x)
{
    return partida->chefao != NULL || gato_na_mira(partida->gatos, linha, x);
}

/* cuspir: cria uma baba do estilo pedido com o dano do cachorro, saindo da
 * boca dele (no sprite) mas na linha pedida; 'recuo' a põe alguns pixels
 * mais para trás. */
static void cuspir(Partida *partida, const Cachorro *cachorro, int linha, float recuo, EstiloBaba estilo)
{
    Vector2 centro = mapa_centro_celula(linha, cachorro->coluna);
    Vector2 boca = { centro.x + 22 - recuo, centro.y + 4 };
    projetil_adicionar(&partida->projeteis, boca, linha, cachorro_info(cachorro->tipo)->dano, estilo);
    som_tocar(SOM_CUSPE);
}

/* produzir_petisco: o Pote de Ração solta um petisco perto de onde aparece
 * a estrela do sprite, caindo à direita da tigela (para não se esconder no
 * monte de ração do desenho), com o barulho da ração caindo. Sempre age. */
static bool produzir_petisco(Partida *partida, Cachorro *cachorro)
{
    Vector2 centro = centro_do(cachorro);
    petisco_adicionar(&partida->petiscos, (Vector2){ centro.x + 18 + rand() % 7 - 3, centro.y + 6 }, centro.y + 20);
    som_tocar(SOM_RACAO);
    return true;
}

/* atirar: o Beagle cospe uma gota de baba, se houver um alvo na linha à frente. */
static bool atirar(Partida *partida, Cachorro *cachorro)
{
    if (!tem_alvo(partida, cachorro->linha, centro_do(cachorro).x)) return false;
    cuspir(partida, cachorro, cachorro->linha, 0, BABA_GOTA);
    return true;
}

/* atirar_duplo: o Dálmata, se houver um alvo na linha à frente, cospe duas
 * bolhas de baba seguidas (a segunda um pouco mais atrás). */
static bool atirar_duplo(Partida *partida, Cachorro *cachorro)
{
    if (!tem_alvo(partida, cachorro->linha, centro_do(cachorro).x)) return false;
    cuspir(partida, cachorro, cachorro->linha, 0, BABA_BOLHAS);
    cuspir(partida, cachorro, cachorro->linha, 14, BABA_BOLHAS);
    return true;
}

/* atirar_gelo: o Husky cospe uma baba congelada, que deixa o gato lento. */
static bool atirar_gelo(Partida *partida, Cachorro *cachorro)
{
    if (!tem_alvo(partida, cachorro->linha, centro_do(cachorro).x)) return false;
    cuspir(partida, cachorro, cachorro->linha, 0, BABA_GELADA);
    return true;
}

/* atirar_tres_linhas: o Dachshund, se houver alvo na sua linha ou numa
 * vizinha, cospe um jato de baba em cada uma dessas três linhas (as que existem). */
static bool atirar_tres_linhas(Partida *partida, Cachorro *cachorro)
{
    float x = centro_do(cachorro).x;
    bool temAlvo = false;
    for (int l = cachorro->linha - 1; l <= cachorro->linha + 1; l++) temAlvo = temAlvo || tem_alvo(partida, l, x);
    if (!temAlvo) return false;

    for (int l = cachorro->linha - 1; l <= cachorro->linha + 1; l++) {
        if (l >= 0 && l < LINHAS_MAPA) cuspir(partida, cachorro, l, 0, BABA_JATO);
    }
    return true;
}

/* morder: o Pastor Alemão late ("AU!") e morde de uma vez todos os gatos da
 * sua linha que estejam até 1,5 célula à frente. Age só se acertar alguém. */
static bool morder(Partida *partida, Cachorro *cachorro)
{
    Vector2 centro = centro_do(cachorro);
    if (gato_ferir_area(partida->gatos, cachorro->linha, centro.x - RAIO_GATO, centro.x + ALCANCE_PASTOR, cachorro_info(cachorro->tipo)->dano) == 0) return false;

    partida_adicionar_latido(partida, centro, "AU!", TAMANHO_CELULA * 0.6f, WHITE);
    return true;
}

/* atacar_da_toca: o Westie, um terrier, passa os primeiros segundos cavando
 * uma toca (só depois disso esta função é chamada). Quando um gato chega à
 * sua célula, ele salta da toca, fere os gatos dali e volta para casa (vida
 * 0, o mapa o remove). Sem gato por perto, continua escondido esperando. */
static bool atacar_da_toca(Partida *partida, Cachorro *cachorro)
{
    Rectangle celula = mapa_retangulo_celula(cachorro->linha, cachorro->coluna);
    float xMax = celula.x + celula.width + RAIO_GATO;
    if (gato_ferir_area(partida->gatos, cachorro->linha, celula.x, xMax, cachorro_info(cachorro->tipo)->dano) == 0) return false;

    cachorro->vida = 0;
    partida_adicionar_efeito(partida, centro_do(cachorro), TAMANHO_CELULA * 0.6f, COR_TERRA);
    som_tocar(SOM_LATIDO);
    return true;
}

/* gritar: o Shiba Inu solta o famoso "grito Shiba" (um latido longo e
 * amarelo) quando há gato na sua linha até 2,5 células à frente: o susto faz
 * esses gatos recuarem meia célula. Age só se assustar alguém. */
static bool gritar(Partida *partida, Cachorro *cachorro)
{
    Vector2 centro = centro_do(cachorro);
    if (gato_empurrar_area(partida->gatos, cachorro->linha, centro.x, centro.x + ALCANCE_GRITO, RECUO_GRITO) == 0) return false;

    partida_adicionar_latido(partida, centro, "AUUUU!", TAMANHO_CELULA * 1.5f, COR_GRITO);
    return true;
}

/* explodir_area: o Rottweiler explode, ferindo os gatos da área 3x3 em volta
 * dele (a linha dele e as duas vizinhas) e o Rei, se a área chegar até ele,
 * e some. */
static bool explodir_area(Partida *partida, Cachorro *cachorro)
{
    Vector2 centro = centro_do(cachorro);
    int dano = cachorro_info(cachorro->tipo)->dano;
    for (int l = cachorro->linha - 1; l <= cachorro->linha + 1; l++) {
        gato_ferir_area(partida->gatos, l, centro.x - RAIO_3X3, centro.x + RAIO_3X3, dano);
    }
    if (partida->chefao != NULL && centro.x + RAIO_3X3 >= FRENTE_CHEFAO_X) chefao_ferir(partida->chefao, dano);
    cachorro->vida = 0;
    partida_adicionar_efeito(partida, centro, RAIO_3X3, COR_EXPLOSAO);
    som_tocar(SOM_EXPLOSAO);
    return true;
}

/* curar_vizinhos: o São Bernardo devolve vida aos cachorros das 8 células em
 * volta dele, sem passar da vida máxima de cada um. Sempre age. */
static bool curar_vizinhos(Partida *partida, Cachorro *cachorro)
{
    int cura = cachorro_info(cachorro->tipo)->dano;
    for (int l = cachorro->linha - 1; l <= cachorro->linha + 1; l++) {
        for (int c = cachorro->coluna - 1; c <= cachorro->coluna + 1; c++) {
            if (l < 0 || l >= LINHAS_MAPA || c < 0 || c >= COLUNAS_MAPA) continue;
            Cachorro *vizinho = partida->mapa.grade[l][c];
            if (vizinho == NULL || vizinho == cachorro) continue;

            int maxima = cachorro_info(vizinho->tipo)->vidaMaxima;
            vizinho->vida = (vizinho->vida + cura > maxima) ? maxima : vizinho->vida + cura;
        }
    }
    partida_adicionar_efeito(partida, centro_do(cachorro), RAIO_3X3, COR_CURA);
    return true;
}

static const Habilidade habilidades[TOTAL_TIPOS_CACHORRO] = {
    [TIPO_POTE_RACAO]    = produzir_petisco,
    [TIPO_BEAGLE]        = atirar,
    [TIPO_BULLDOG]       = NULL,                // muralha: só aguenta as mordidas
    [TIPO_DALMATA]       = atirar_duplo,
    [TIPO_HUSKY]         = atirar_gelo,
    [TIPO_PASTOR_ALEMAO] = morder,
    [TIPO_WESTIE]        = atacar_da_toca,
    [TIPO_SAO_BERNARDO]  = curar_vizinhos,
    [TIPO_DACHSHUND]     = atirar_tres_linhas,
    [TIPO_ROTTWEILER]    = explodir_area,
    [TIPO_SHIBA]         = gritar,
};

/* habilidade_usar: busca na tabela o ponteiro da função da raça e a chama.
 * Retorna true se o cachorro agiu, ou false se ele não tem habilidade
 * (NULL) ou ainda está esperando um alvo. */
bool habilidade_usar(Partida *partida, Cachorro *cachorro)
{
    Habilidade habilidade = habilidades[cachorro->tipo];
    return habilidade != NULL && habilidade(partida, cachorro);
}
