#ifndef SOM_H
#define SOM_H

/* som.h - Efeitos sonoros (cliques, cuspes, latidos, miados, sirene...).
 * Cada efeito é um WAV em assets/sons/, carregado uma vez numa TABELA. Se
 * um arquivo faltar (ou não houver áudio), aquele efeito só fica mudo. */

#include "raylib.h"

typedef enum IdSom {
    SOM_CLIQUE,        // botões dos menus
    SOM_CARTA,         // cartas, pá, luva e 2x durante a partida
    SOM_INICIAR,       // a partida começa
    SOM_COLOCAR,       // cachorro posto no campo
    SOM_PA,            // cachorro tirado com a pá
    SOM_LUVA,          // cachorro pego ou mudado de lugar com a luva
    SOM_PETISCO,       // petisco coletado
    SOM_RACAO,         // o Pote de Ração soltando um petisco
    SOM_CUSPE,         // atirador cuspindo baba
    SOM_LATIDO,        // Pastor Alemão, Shiba Inu e Westie
    SOM_EXPLOSAO,      // Rottweiler
    SOM_ZUMBIDO,       // gato zumbi entrando no campo (gemido e passos)
    SOM_MORDIDA,       // gato mordendo um cachorro
    SOM_GATO_MORRE,    // gato derrotado
    SOM_RUGIDO,        // o Rei dos Gatos
    SOM_ALERTA,        // sirene: uma horda está chegando
    SOM_VITORIA,
    SOM_DERROTA,
    TOTAL_SONS
} IdSom;

void  som_carregar_todos(void);           // carrega os WAVs (depois de abrir o áudio)
void  som_descarregar_todos(void);        // libera os sons
void  som_tocar(IdSom id);                // toca o efeito (recomeça se já estiver tocando)
void  som_definir_volume(float volume);   // volume dos efeitos, de 0 a 1
float som_volume(void);                   // volume atual dos efeitos

#endif
