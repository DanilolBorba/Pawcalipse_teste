/* ui.c - Elementos de interface reutilizados por todas as telas. */

#include "ui.h"

#include <string.h>

/* ui_botao_clicado: verifica se o botão esquerdo do mouse foi pressionado
 * sobre o botão neste frame. Retorna true em caso de clique. */
bool ui_botao_clicado(const Botao *botao, Vector2 mouse)
{
    return CheckCollisionPointRec(mouse, botao->area) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

/* ui_desenhar_botao: desenha o botão com uma sombra "dura" de pixel-art.
 * Com o mouse em cima, ele afunda 1 pixel e muda de cor. Se o texto não
 * couber na fonte média, usa a pequena. */
void ui_desenhar_botao(const Botao *botao, Vector2 mouse)
{
    bool emCima = CheckCollisionPointRec(mouse, botao->area);
    Rectangle r = botao->area;

    DrawRectangle(r.x + 2, r.y + 2, r.width, r.height, COR_SOMBRA);
    if (emCima) {
        r.x += 1;
        r.y += 1;
    }
    DrawRectangleRec(r, emCima ? COR_BOTAO_HOVER : COR_BOTAO);
    DrawRectangleLinesEx(r, 2, COR_BORDA);

    int tamanho = (MeasureText(botao->texto, FONTE_MEDIA) > r.width - 8) ? FONTE_PEQUENA : FONTE_MEDIA;
    ui_texto_centro(botao->texto, r.x + r.width / 2, r.y + (r.height - tamanho) / 2, tamanho,
                    emCima ? COR_INFECTADO : COR_TEXTO);
}

/* ui_desenhar_painel: desenha um retângulo escuro com borda, usado como
 * fundo de caixas e janelas. */
void ui_desenhar_painel(Rectangle area)
{
    DrawRectangleRec(area, COR_PAINEL);
    DrawRectangleLinesEx(area, 2, COR_BORDA);
}

/* ui_texto: escreve o texto com o canto superior esquerdo em (x, y) e uma
 * sombra deslocada proporcional ao tamanho da fonte, para ficar legível em
 * qualquer fundo. */
void ui_texto(const char *texto, int x, int y, int tamanho, Color cor)
{
    int sombra = tamanho / 10;
    DrawText(texto, x + sombra, y + sombra, tamanho, COR_SOMBRA);
    DrawText(texto, x, y, tamanho, cor);
}

/* ui_texto_centro: igual a ui_texto, mas centraliza o texto em torno de
 * centroX (use LARGURA_VIRTUAL / 2 para centralizar na tela). */
void ui_texto_centro(const char *texto, float centroX, int y, int tamanho, Color cor)
{
    ui_texto(texto, centroX - MeasureText(texto, tamanho) / 2, y, tamanho, cor);
}

/* ui_desenhar_osso: desenha um osso (haste + 4 pontas redondas) centrado em
 * 'centro'. Desenha duas vezes: primeiro o contorno marrom, um pixel maior,
 * e depois o preenchimento na cor pedida. */
void ui_desenhar_osso(Vector2 centro, float escala, Color cor)
{
    Color cores[2] = { { 92, 62, 42, cor.a }, cor };
    float e = escala;

    for (int k = 0; k < 2; k++) {
        float borda = (k == 0) ? 1.0f : 0.0f;
        DrawRectangleRec((Rectangle){ centro.x - 7 * e, centro.y - 2 * e - borda, 14 * e, 4 * e + 2 * borda }, cores[k]);
        for (int lado = -1; lado <= 1; lado += 2) {
            DrawCircleV((Vector2){ centro.x + lado * 7 * e, centro.y - 2.5f * e }, 3 * e + borda, cores[k]);
            DrawCircleV((Vector2){ centro.x + lado * 7 * e, centro.y + 2.5f * e }, 3 * e + borda, cores[k]);
        }
    }
}

/* ui_desenhar_barra_vida: barra fina centrada em 'centro', com a largura
 * dada. 'fracao' vai de 0 (vazia) a 1 (cheia); a parte cheia é verde e fica
 * vermelha quando a vida cai abaixo da metade. */
void ui_desenhar_barra_vida(Vector2 centro, float largura, float fracao)
{
    Rectangle fundo = { centro.x - largura / 2, centro.y - 2, largura, 4 };
    DrawRectangleRec(fundo, COR_SOMBRA);
    DrawRectangle(fundo.x, fundo.y, largura * fracao, fundo.height, (fracao > 0.5f) ? COR_INFECTADO : COR_ALERTA);
}

/* ui_desenhar_pixel_art: desenha uma figura guardada como MATRIZ de
 * caracteres (um vetor de strings, um caractere por pixel), com o canto
 * superior esquerdo em 'canto' e cada pixel ampliado 'escala' vezes. Cada
 * caractere é procurado em 'simbolos' e o pixel recebe a cor de mesma
 * posição em 'cores'; os que não estão lá (como '.') ficam transparentes. */
void ui_desenhar_pixel_art(const char *figura[], int linhas, Vector2 canto, int escala,
                           const char *simbolos, const Color cores[])
{
    for (int l = 0; l < linhas; l++) {
        for (int c = 0; figura[l][c] != '\0'; c++) {
            const char *achado = strchr(simbolos, figura[l][c]);
            if (achado != NULL) DrawRectangle(canto.x + c * escala, canto.y + l * escala, escala, escala, cores[achado - simbolos]);
        }
    }
}
