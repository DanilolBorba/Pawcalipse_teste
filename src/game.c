/* game.c - Máquina de estados das telas: menu, instruções, partida e pausa. */

#include "game.h"
#include "cenario.h"
#include "config.h"
#include "ui.h"

#include <math.h>
#include <stdlib.h>

enum { BOTAO_JOGAR, BOTAO_INSTRUCOES, BOTAO_SAIR, TOTAL_BOTOES_MENU };

static const Botao botoesMenu[TOTAL_BOTOES_MENU] = {
    { { 240, 150, 160, 30 }, "Jogar"      },
    { { 240, 190, 160, 30 }, "Instruções" },
    { { 240, 230, 160, 30 }, "Sair"       },
};
static const Botao botaoVoltar = { { 240, 312, 160, 30 }, "Voltar" };
static const Botao botaoContinuar = { { 220, 160, 200, 30 }, "Continuar" };
static const Botao botaoSairPartida = { { 220, 200, 200, 30 }, "Menu Principal" };

/* area_da_tela: calcula onde a tela virtual aparece dentro da janela. Usa
 * escala inteira (2x, 3x...) para todos os pixels terem o mesmo tamanho;
 * o espaço que sobra vira borda preta. Retorna o retângulo de destino. */
static Rectangle area_da_tela(void)
{
    float escala = fminf((float)GetScreenWidth() / LARGURA_VIRTUAL, (float)GetScreenHeight() / ALTURA_VIRTUAL);
    if (escala >= 1.0f) escala = floorf(escala);

    float largura = LARGURA_VIRTUAL * escala, altura = ALTURA_VIRTUAL * escala;
    return (Rectangle){ (GetScreenWidth() - largura) / 2, (GetScreenHeight() - altura) / 2, largura, altura };
}

/* atualizar_menu: troca de tela conforme o botão clicado no menu. */
static void atualizar_menu(Jogo *jogo)
{
    if (ui_botao_clicado(&botoesMenu[BOTAO_JOGAR], jogo->mouse))      jogo->estado = ESTADO_JOGANDO;
    if (ui_botao_clicado(&botoesMenu[BOTAO_INSTRUCOES], jogo->mouse)) jogo->estado = ESTADO_INSTRUCOES;
    if (ui_botao_clicado(&botoesMenu[BOTAO_SAIR], jogo->mouse))       jogo->estado = ESTADO_SAIR;
}

/* desenhar_menu: cenário, título balançando, subtítulo, botões e versão. */
static void desenhar_menu(const Jogo *jogo)
{
    cenario_desenhar_menu(jogo->tempo);
    titulo_desenhar(40 + (int)(sinf(jogo->tempo * 2.0f) * 3.0f), jogo->tempo);
    ui_texto_centro("Cães vs. Gatos Zumbis", LARGURA_VIRTUAL / 2, 104, FONTE_MEDIA, COR_TEXTO);

    for (int i = 0; i < TOTAL_BOTOES_MENU; i++) ui_desenhar_botao(&botoesMenu[i], jogo->mouse);
    ui_texto(VERSAO_JOGO, 6, ALTURA_VIRTUAL - 14, FONTE_PEQUENA, COR_TEXTO);
}

/* desenhar_instrucoes: painel com o texto de como jogar e o botão Voltar. */
static void desenhar_instrucoes(const Jogo *jogo)
{
    static const char *linhas[] = {
        "Os Gatos Zumbis avançam pela direita rumo à sua casa.",
        "",
        "- Clique em uma carta de cachorro e depois em um espaço do campo.",
        "- Cada cachorro custa petiscos. Você começa com 200.",
        "- O pote de ração gera petiscos; clique neles para coletar.",
        "- Use a pá para remover um cachorro do campo.",
        "- Sobreviva às 4 hordas de cada fase. Na 5ª fase, enfrente o chefão!",
        "- Se um gato chegar à casa, você perde a fase.",
        "",
        "ESC: pausar a partida        F11: tela cheia",
    };

    cenario_desenhar_menu(jogo->tempo);
    ui_desenhar_painel((Rectangle){ 50, 30, 540, 264 });
    ui_texto_centro("Como Jogar", LARGURA_VIRTUAL / 2, 44, FONTE_MEDIA, COR_INFECTADO);
    for (int i = 0; i < (int)(sizeof(linhas) / sizeof(linhas[0])); i++) {
        ui_texto(linhas[i], 70, 84 + i * 18, FONTE_PEQUENA, COR_TEXTO);
    }
    ui_desenhar_botao(&botaoVoltar, jogo->mouse);
}

/* desenhar_jogando: esboço da partida (o campo de batalha entra na Etapa 2). */
static void desenhar_jogando(const Jogo *jogo)
{
    cenario_desenhar_menu(jogo->tempo);
    ui_texto_centro("Partida em construção", LARGURA_VIRTUAL / 2, 250, FONTE_MEDIA, COR_TEXTO);
    ui_texto_centro("ESC para pausar", LARGURA_VIRTUAL / 2, 280, FONTE_PEQUENA, COR_INFECTADO);
}

/* desenhar_pausa: a partida congelada ao fundo, escurecida, com os botões
 * Continuar e Menu Principal por cima. */
static void desenhar_pausa(const Jogo *jogo)
{
    desenhar_jogando(jogo);
    DrawRectangle(0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL, Fade(BLACK, 0.6f));
    ui_texto_centro("PAUSADO", LARGURA_VIRTUAL / 2, 100, FONTE_GRANDE, COR_TEXTO);
    ui_desenhar_botao(&botaoContinuar, jogo->mouse);
    ui_desenhar_botao(&botaoSairPartida, jogo->mouse);
}

/* jogo_criar: aloca a estrutura do jogo com malloc e cria a tela virtual com
 * filtro POINT (ao ampliar, cada pixel vira um bloco, sem borrar).
 * Retorna o ponteiro para o jogo, ou NULL se faltar memória. */
Jogo *jogo_criar(void)
{
    Jogo *jogo = malloc(sizeof(Jogo));
    if (jogo == NULL) return NULL;

    jogo->estado = ESTADO_MENU;
    jogo->mouse = (Vector2){ 0, 0 };
    jogo->tempo = 0.0f;
    jogo->tela = LoadRenderTexture(LARGURA_VIRTUAL, ALTURA_VIRTUAL);
    SetTextureFilter(jogo->tela.texture, TEXTURE_FILTER_POINT);
    return jogo;
}

/* jogo_atualizar: avança o tempo, converte o mouse para a tela virtual
 * (SetMouseOffset/SetMouseScale desfazem a escala e as bordas), trata o
 * F11 e executa a lógica da tela atual. */
void jogo_atualizar(Jogo *jogo)
{
    Rectangle area = area_da_tela();
    SetMouseOffset(-(int)area.x, -(int)area.y);
    SetMouseScale(LARGURA_VIRTUAL / area.width, ALTURA_VIRTUAL / area.height);
    jogo->mouse = GetMousePosition();
    jogo->tempo += GetFrameTime();

    if (IsKeyPressed(KEY_F11)) ToggleBorderlessWindowed();

    switch (jogo->estado) {
        case ESTADO_MENU:
            atualizar_menu(jogo);
            break;
        case ESTADO_INSTRUCOES:
            if (ui_botao_clicado(&botaoVoltar, jogo->mouse) || IsKeyPressed(KEY_ESCAPE)) jogo->estado = ESTADO_MENU;
            break;
        case ESTADO_JOGANDO:
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) jogo->estado = ESTADO_PAUSA;
            break;
        case ESTADO_PAUSA:
            if (ui_botao_clicado(&botaoContinuar, jogo->mouse) || IsKeyPressed(KEY_ESCAPE)) jogo->estado = ESTADO_JOGANDO;
            if (ui_botao_clicado(&botaoSairPartida, jogo->mouse)) jogo->estado = ESTADO_MENU;
            break;
        case ESTADO_SAIR:
            break;
    }
}

/* jogo_desenhar: desenha a tela atual na tela virtual e depois a amplia para
 * a janela. A altura da origem é negativa porque a RenderTexture fica de
 * cabeça para baixo no OpenGL. */
void jogo_desenhar(const Jogo *jogo)
{
    BeginTextureMode(jogo->tela);
    ClearBackground(BLACK);
    switch (jogo->estado) {
        case ESTADO_MENU:       desenhar_menu(jogo);       break;
        case ESTADO_INSTRUCOES: desenhar_instrucoes(jogo); break;
        case ESTADO_JOGANDO:    desenhar_jogando(jogo);    break;
        case ESTADO_PAUSA:      desenhar_pausa(jogo);      break;
        case ESTADO_SAIR:                                  break;
    }
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);
    Rectangle origem = { 0, 0, LARGURA_VIRTUAL, -ALTURA_VIRTUAL };
    DrawTexturePro(jogo->tela.texture, origem, area_da_tela(), (Vector2){ 0, 0 }, 0.0f, WHITE);
    EndDrawing();
}

/* jogo_destruir: libera a tela virtual e a memória do próprio jogo. */
void jogo_destruir(Jogo *jogo)
{
    if (jogo == NULL) return;
    UnloadRenderTexture(jogo->tela);
    free(jogo);
}
