#ifndef UI_H
#define UI_H

/* ui.h - Botões, textos, painéis e a paleta de cores da interface.
 * Todas as coordenadas são da tela virtual (640x360). */

#include "raylib.h"
#include <stdbool.h>

#define COR_TEXTO        (Color){ 240, 232, 214, 255 }
#define COR_SOMBRA       (Color){  20,  12,  20, 255 }
#define COR_INFECTADO    (Color){ 132, 222,  76, 255 }
#define COR_PETISCO      (Color){ 232, 160,  60, 255 }
#define COR_OSSO         (Color){ 240, 230, 204, 255 }
#define COR_ALERTA       (Color){ 255,  96,  80, 255 }
#define COR_BLOQUEADO    (Color){ 120, 112, 124, 255 }
#define COR_SANGUE       (Color){ 178,  20,  26, 255 }
#define COR_SANGUE_ESC   (Color){  90,   6,  12, 255 }
#define COR_BOTAO        (Color){  92,  58,  40, 255 }
#define COR_BOTAO_HOVER  (Color){ 140,  90,  52, 255 }
#define COR_BORDA        (Color){  32,  18,  14, 255 }
#define COR_PAINEL       (Color){  30,  20,  32, 230 }

#define FONTE_PEQUENA  10   // tamanhos nítidos com a fonte padrão (múltiplos de 10)
#define FONTE_MEDIA    20
#define FONTE_GRANDE   40

typedef struct Botao {
    Rectangle   area;    // posição e tamanho
    const char *texto;   // rótulo no centro
} Botao;

bool ui_botao_clicado(const Botao *botao, Vector2 mouse);   // true no frame em que o botão é clicado
void ui_desenhar_botao(const Botao *botao, Vector2 mouse);  // desenha o botão (mais claro com o mouse em cima)
void ui_desenhar_painel(Rectangle area);                    // fundo escuro com borda
void ui_texto(const char *texto, int x, int y, int tamanho, Color cor);                // texto com sombra
void ui_texto_centro(const char *texto, float centroX, int y, int tamanho, Color cor);  // texto com sombra centralizado em centroX
void ui_desenhar_osso(Vector2 centro, float escala, Color cor);                         // ícone de osso (petisco)

#endif
