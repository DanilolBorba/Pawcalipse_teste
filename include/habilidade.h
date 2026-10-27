#ifndef HABILIDADE_H
#define HABILIDADE_H

/* habilidade.h - O que cada raça faz no campo. Uma tabela de PONTEIROS DE
 * FUNÇÃO liga cada raça à função da sua habilidade. */

#include "partida.h"
#include <stdbool.h>

typedef bool (*Habilidade)(Partida *partida, Cachorro *cachorro);   // retorna true se agiu (false = esperando alvo)

bool habilidade_usar(Partida *partida, Cachorro *cachorro);   // executa a habilidade da raça; true se agiu

#endif
