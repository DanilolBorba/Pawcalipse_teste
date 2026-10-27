#ifndef HORDA_H
#define HORDA_H

/* horda.h - As 4 hordas de cada fase. Quantos gatos de cada tipo vêm em
 * cada horda fica numa MATRIZ [fase][horda][tipo de gato]. */

#include "enemy.h"
#include <stdbool.h>

#define HORDAS_POR_FASE  4

typedef struct Hordas {
    int   fase;                       // 1 a 5 (a 5 usa a tabela da 4 até o chefão existir)
    int   atual;                      // horda em andamento (0 a 3); HORDAS_POR_FASE = todas vencidas
    int   faltam[TOTAL_TIPOS_GATO];   // gatos de cada tipo que ainda vão sair nesta horda
    int   totalFase;                  // gatos da fase inteira
    int   enviados;                   // gatos da fase que já saíram
    float timerGato;                  // segundos até o próximo gato sair
    float timerPausa;                 // segundos de preparação/pausa antes da horda atual
} Hordas;

void horda_iniciar(Hordas *hordas, int fase);                       // prepara as hordas da fase
void horda_atualizar(Hordas *hordas, Gato **gatos, float dt);       // manda os gatos e avança as hordas
bool horda_terminou(const Hordas *hordas);                          // true se as 4 hordas foram vencidas
int  horda_numero(const Hordas *hordas);                            // horda atual, de 1 a 4
float horda_progresso(const Hordas *hordas);                        // fração dos gatos da fase que já saíram (0 a 1)
const char *horda_anuncio(const Hordas *hordas);                    // texto do anúncio antes da horda (NULL se nenhum)

#endif
