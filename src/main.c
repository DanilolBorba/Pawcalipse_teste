/* PawPocalypse - Cães vs. Gatos Zumbis
 * main.c - Ponto de entrada do programa. */

#include "raylib.h"
#include "config.h"
#include "game.h"

#include <stdio.h>

/* main: abre a janela (redimensionável, 60 FPS), desliga o "ESC fecha o
 * jogo" da raylib (aqui o ESC pausa), muda a pasta de trabalho para a do
 * executável (para achar assets/) e roda o loop principal até o jogador
 * sair ou fechar a janela. */
int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(LARGURA_JANELA, ALTURA_JANELA, TITULO_JOGO);
    SetWindowMinSize(LARGURA_VIRTUAL, ALTURA_VIRTUAL);
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    ChangeDirectory(GetApplicationDirectory());

    Jogo *jogo = jogo_criar();
    if (jogo == NULL) {
        fprintf(stderr, "Erro: memória insuficiente para iniciar o jogo.\n");
        CloseWindow();
        return 1;
    }

    while (!WindowShouldClose() && jogo->estado != ESTADO_SAIR) {
        jogo_atualizar(jogo);
        jogo_desenhar(jogo);
    }

    jogo_destruir(jogo);
    CloseWindow();
    return 0;
}
