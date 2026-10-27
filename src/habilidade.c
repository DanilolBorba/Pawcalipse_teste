/* habilidade.c - As habilidades das raças e a tabela de ponteiros de função
 * que liga cada raça à sua. */

#include "habilidade.h"

#include <stdlib.h>

#define ALCANCE_PASTOR  (1.5f * TAMANHO_CELULA)   // até onde a mordida do Pastor Alemão alcança
#define RAIO_3X3        (1.5f * TAMANHO_CELULA)   // metade da largura da área 3x3

static const Color COR_EXPLOSAO = { 255, 150,  40, 255 };
static const Color COR_CURA     = { 130, 230, 120, 255 };

/* centro_do: centro da célula onde o cachorro está. */
static Vector2 centro_do(const Cachorro *cachorro)
{
    return mapa_centro_celula(cachorro->linha, cachorro->coluna);
}

/* lancar_bolinha: cria uma bolinha com o dano do cachorro, saindo da boca
 * dele mas na linha pedida; 'recuo' a põe alguns pixels mais para trás. */
static void lancar_bolinha(Partida *partida, const Cachorro *cachorro, int linha, float recuo, bool gelada)
{
    Vector2 centro = mapa_centro_celula(linha, cachorro->coluna);
    Vector2 boca = { centro.x + 16 - recuo, centro.y - 4 };
    projetil_adicionar(&partida->projeteis, boca, linha, cachorro_info(cachorro->tipo)->dano, gelada);
}

/* produzir_petisco: o Pote de Ração solta um petisco perto de onde aparece
 * a estrela do sprite, caindo à direita da tigela (para não se esconder no
 * monte de ração do desenho). Sempre age. */
static bool produzir_petisco(Partida *partida, Cachorro *cachorro)
{
    Vector2 centro = centro_do(cachorro);
    petisco_adicionar(&partida->petiscos, (Vector2){ centro.x + 18 + rand() % 7 - 3, centro.y + 6 }, centro.y + 20);
    return true;
}

/* atirar: o Beagle joga uma bolinha, se houver um gato na linha à frente. */
static bool atirar(Partida *partida, Cachorro *cachorro)
{
    if (!gato_na_mira(partida->gatos, cachorro->linha, centro_do(cachorro).x)) return false;
    lancar_bolinha(partida, cachorro, cachorro->linha, 0, false);
    return true;
}

/* atirar_duplo: o Dálmata atira como o Beagle e logo depois (uma bolinha
 * um pouco mais atrás) atira de novo. */
static bool atirar_duplo(Partida *partida, Cachorro *cachorro)
{
    if (!atirar(partida, cachorro)) return false;
    lancar_bolinha(partida, cachorro, cachorro->linha, 14, false);
    return true;
}

/* atirar_gelo: o Husky joga uma bolinha gelada, que deixa o gato lento. */
static bool atirar_gelo(Partida *partida, Cachorro *cachorro)
{
    if (!gato_na_mira(partida->gatos, cachorro->linha, centro_do(cachorro).x)) return false;
    lancar_bolinha(partida, cachorro, cachorro->linha, 0, true);
    return true;
}

/* atirar_tres_linhas: o Dachshund, se houver gato na sua linha ou numa
 * vizinha, joga uma bolinha em cada uma dessas três linhas (as que existem). */
static bool atirar_tres_linhas(Partida *partida, Cachorro *cachorro)
{
    float x = centro_do(cachorro).x;
    bool temAlvo = false;
    for (int l = cachorro->linha - 1; l <= cachorro->linha + 1; l++) temAlvo = temAlvo || gato_na_mira(partida->gatos, l, x);
    if (!temAlvo) return false;

    for (int l = cachorro->linha - 1; l <= cachorro->linha + 1; l++) {
        if (l >= 0 && l < LINHAS_MAPA) lancar_bolinha(partida, cachorro, l, 0, false);
    }
    return true;
}

/* morder: o Pastor Alemão morde de uma vez todos os gatos da sua linha que
 * estejam até 1,5 célula à frente. Age só se acertar alguém. */
static bool morder(Partida *partida, Cachorro *cachorro)
{
    float x = centro_do(cachorro).x;
    return gato_ferir_area(partida->gatos, cachorro->linha, x - RAIO_GATO, x + ALCANCE_PASTOR, cachorro_info(cachorro->tipo)->dano) > 0;
}

/* explodir_mina: o Chihuahua (só chamado depois de armado) explode quando um
 * gato chega à sua célula: fere os gatos dali e some (vida 0, o mapa o
 * remove). Sem gato por perto, continua esperando. */
static bool explodir_mina(Partida *partida, Cachorro *cachorro)
{
    Rectangle celula = mapa_retangulo_celula(cachorro->linha, cachorro->coluna);
    float xMax = celula.x + celula.width + RAIO_GATO;
    if (gato_ferir_area(partida->gatos, cachorro->linha, celula.x, xMax, cachorro_info(cachorro->tipo)->dano) == 0) return false;

    cachorro->vida = 0;
    partida_adicionar_efeito(partida, centro_do(cachorro), TAMANHO_CELULA * 0.6f, COR_EXPLOSAO);
    return true;
}

/* explodir_area: o Rottweiler explode, ferindo os gatos da área 3x3 em volta
 * dele (a linha dele e as duas vizinhas), e some. */
static bool explodir_area(Partida *partida, Cachorro *cachorro)
{
    Vector2 centro = centro_do(cachorro);
    for (int l = cachorro->linha - 1; l <= cachorro->linha + 1; l++) {
        gato_ferir_area(partida->gatos, l, centro.x - RAIO_3X3, centro.x + RAIO_3X3, cachorro_info(cachorro->tipo)->dano);
    }
    cachorro->vida = 0;
    partida_adicionar_efeito(partida, centro, RAIO_3X3, COR_EXPLOSAO);
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
    [TIPO_CHIHUAHUA]     = explodir_mina,
    [TIPO_SAO_BERNARDO]  = curar_vizinhos,
    [TIPO_DACHSHUND]     = atirar_tres_linhas,
    [TIPO_ROTTWEILER]    = explodir_area,
};

/* habilidade_usar: busca na tabela o ponteiro da função da raça e a chama.
 * Retorna true se o cachorro agiu, ou false se ele não tem habilidade
 * (NULL) ou ainda está esperando um alvo. */
bool habilidade_usar(Partida *partida, Cachorro *cachorro)
{
    Habilidade habilidade = habilidades[cachorro->tipo];
    return habilidade != NULL && habilidade(partida, cachorro);
}
