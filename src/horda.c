/* horda.c - Tabela das hordas e o envio dos gatos de cada horda. */

#include "horda.h"
#include "map.h"

#include <stdlib.h>

#define FASES_COM_HORDAS    4       // a fase 5 usa a tabela da 4 até o chefão existir (Etapa 7)
#define PREPARACAO          20.0f   // segundos antes da 1ª horda
#define PAUSA_ENTRE_HORDAS  8.0f
#define INTERVALO_INICIAL   6.0f    // segundos entre gatos na 1ª horda (1 a menos a cada horda)
#define DURACAO_ANUNCIO     3.0f    // segundos finais da pausa em que o anúncio aparece

static const int tabela[FASES_COM_HORDAS][HORDAS_POR_FASE][TOTAL_TIPOS_GATO] = {
    /*               padrão caixa lata siamês persa telhado maine */
    { /* fase 1 */ {     4,    0,   0,    0,     0,    0,     0 },
                   {     6,    1,   0,    0,     0,    0,     0 },
                   {     7,    2,   0,    0,     0,    0,     0 },
                   {    10,    4,   0,    0,     0,    0,     0 } },
    { /* fase 2 */ {     5,    1,   0,    0,     0,    0,     0 },
                   {     6,    2,   1,    1,     0,    0,     0 },
                   {     7,    2,   1,    2,     0,    0,     0 },
                   {    10,    4,   2,    3,     0,    0,     0 } },
    { /* fase 3 */ {     6,    1,   0,    0,     1,    0,     0 },
                   {     6,    2,   1,    1,     1,    1,     0 },
                   {     7,    3,   2,    2,     1,    2,     0 },
                   {    10,    4,   3,    3,     2,    3,     0 } },
    { /* fase 4 */ {     6,    2,   0,    1,     0,    1,     0 },
                   {     7,    3,   1,    2,     1,    1,     0 },
                   {     8,    3,   2,    2,     2,    2,     1 },
                   {    12,    5,   3,    3,     3,    3,     2 } },
};

/* indice_da_fase: posição da fase na tabela (a 5 usa a da 4, por enquanto). */
static int indice_da_fase(int fase)
{
    return ((fase > FASES_COM_HORDAS) ? FASES_COM_HORDAS : fase) - 1;
}

/* carregar_horda: copia da tabela quantos gatos de cada tipo a horda atual
 * vai mandar e zera o timer, para o 1º gato sair assim que a pausa acabar. */
static void carregar_horda(Hordas *hordas)
{
    for (int t = 0; t < TOTAL_TIPOS_GATO; t++) hordas->faltam[t] = tabela[indice_da_fase(hordas->fase)][hordas->atual][t];
    hordas->timerGato = 0.0f;
}

/* horda_iniciar: zera o controle, soma quantos gatos a fase tem no total
 * (para a barra de progresso), carrega a 1ª horda e começa a preparação. */
void horda_iniciar(Hordas *hordas, int fase)
{
    *hordas = (Hordas){ 0 };
    hordas->fase = fase;
    hordas->timerPausa = PREPARACAO;
    for (int h = 0; h < HORDAS_POR_FASE; h++) {
        for (int t = 0; t < TOTAL_TIPOS_GATO; t++) hordas->totalFase += tabela[indice_da_fase(fase)][h][t];
    }
    carregar_horda(hordas);
}

/* gatos_faltando: quantos gatos da horda atual ainda não saíram. */
static int gatos_faltando(const Hordas *hordas)
{
    int total = 0;
    for (int t = 0; t < TOTAL_TIPOS_GATO; t++) total += hordas->faltam[t];
    return total;
}

/* sortear_tipo: sorteia o tipo do próximo gato entre os que faltam, com
 * chance proporcional à quantidade de cada tipo, e o desconta da horda. */
static TipoGato sortear_tipo(Hordas *hordas)
{
    int sorteio = rand() % gatos_faltando(hordas);
    int tipo = 0;
    while (sorteio >= hordas->faltam[tipo]) {
        sorteio -= hordas->faltam[tipo];
        tipo++;
    }
    hordas->faltam[tipo]--;
    return (TipoGato)tipo;
}

/* horda_atualizar: durante a pausa, só conta o tempo. Depois, manda os
 * gatos da horda um a um, numa linha aleatória, cada vez mais depressa
 * (6, 5, 4 e 3 s entre eles). Quando todos já saíram e o campo fica limpo,
 * passa para a próxima horda, com uma pausa antes. */
void horda_atualizar(Hordas *hordas, Gato **gatos, float dt)
{
    if (horda_terminou(hordas)) return;
    if (hordas->timerPausa > 0.0f) {
        hordas->timerPausa -= dt;
        return;
    }

    if (gatos_faltando(hordas) > 0) {
        hordas->timerGato -= dt;
        if (hordas->timerGato > 0.0f) return;
        gato_adicionar(gatos, sortear_tipo(hordas), rand() % LINHAS_MAPA);
        hordas->enviados++;
        hordas->timerGato = INTERVALO_INICIAL - hordas->atual;
        return;
    }
    if (*gatos != NULL) return;

    hordas->atual++;
    if (horda_terminou(hordas)) return;
    carregar_horda(hordas);
    hordas->timerPausa = PAUSA_ENTRE_HORDAS;
}

/* horda_terminou: retorna true quando as 4 hordas já foram vencidas. */
bool horda_terminou(const Hordas *hordas)
{
    return hordas->atual >= HORDAS_POR_FASE;
}

/* horda_numero: número da horda atual, de 1 a 4 (para mostrar na tela). */
int horda_numero(const Hordas *hordas)
{
    return horda_terminou(hordas) ? HORDAS_POR_FASE : hordas->atual + 1;
}

/* horda_progresso: fração dos gatos da fase que já saíram, de 0 a 1. */
float horda_progresso(const Hordas *hordas)
{
    return (hordas->totalFase > 0) ? (float)hordas->enviados / hordas->totalFase : 0.0f;
}

/* horda_anuncio: nos últimos segundos de cada pausa, retorna o texto que
 * anuncia a horda que vem; fora disso, retorna NULL. */
const char *horda_anuncio(const Hordas *hordas)
{
    if (horda_terminou(hordas) || hordas->timerPausa <= 0.0f || hordas->timerPausa > DURACAO_ANUNCIO) return NULL;
    if (hordas->atual == 0) return "OS GATOS ESTÃO CHEGANDO!";
    if (hordas->atual == HORDAS_POR_FASE - 1) return "ÚLTIMA HORDA!";
    return TextFormat("HORDA %d!", hordas->atual + 1);
}
