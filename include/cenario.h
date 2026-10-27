#ifndef CENARIO_H
#define CENARIO_H

/* cenario.h - Cenário de fundo (pôr do sol sobre a cidade em ruínas) e o título. */

#include "raylib.h"

unsigned int cenario_ruido(int a, int b, int c);     // número "aleatório" fixo para cada entrada
void cenario_desenhar_menu(float tempo);             // céu, sol, prédios e chão do menu
void cenario_desenhar_ceu(int altura, float tempo);  // céu do pôr do sol de y = 0 até 'altura'
void cenario_desenhar_skyline_distante(int base);    // silhueta dos prédios distantes
void titulo_desenhar(int y, float tempo);            // "PawPocalypse" verde com sangue pingando

#endif
