/* game.c - Máquina de estados das telas: menu, saves, fases, instruções,
 * partida e pausa. */

#include "game.h"
#include "cenario.h"
#include "config.h"
#include "ui.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define RAIO_FASE 28

enum { BOTAO_JOGAR, BOTAO_TUTORIAL, BOTAO_INSTRUCOES, BOTAO_SAIR, TOTAL_BOTOES_MENU };

static const Botao botoesMenu[TOTAL_BOTOES_MENU] = {
    { { 240, 136, 160, 30 }, "Jogar"      },
    { { 240, 172, 160, 30 }, "Tutorial"   },
    { { 240, 208, 160, 30 }, "Instruções" },
    { { 240, 244, 160, 30 }, "Sair"       },
};
static const Botao botaoVoltar = { { 240, 312, 160, 30 }, "Voltar" };
static const Botao botaoContinuar = { { 220, 160, 200, 30 }, "Continuar" };
static const Botao botaoSairPartida = { { 220, 200, 200, 30 }, "Sair da Partida" };
static const Botao botaoDerrota = { { 220, 200, 200, 30 }, "Voltar ao Mapa" };

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

/* texto_tempo: formata segundos como "mm:ss". */
static const char *texto_tempo(float segundos)
{
    int s = (int)segundos;
    return TextFormat("%02d:%02d", s / 60, s % 60);
}

/* gravar_saves: grava os saves no arquivo e avisa no terminal se falhar. */
static void gravar_saves(const Jogo *jogo)
{
    if (!save_gravar_todos(jogo->saves)) fprintf(stderr, "Aviso: não foi possível gravar %s\n", ARQUIVO_SAVES);
}

/* iniciar_partida: cria a partida da fase pedida (FASE_TUTORIAL para o
 * tutorial) e vai para a tela de jogo. Se faltar memória, fica onde está. */
static void iniciar_partida(Jogo *jogo, int fase)
{
    jogo->partida = partida_criar(fase);
    if (jogo->partida != NULL) jogo->estado = ESTADO_JOGANDO;
}

/* encerrar_partida: soma o tempo jogado ao save atual e, se 'venceu', marca
 * a fase como concluída e libera a próxima; grava o arquivo, destrói a
 * partida e volta para o mapa de fases (ou para o menu, se era o tutorial). */
static void encerrar_partida(Jogo *jogo, bool venceu)
{
    Partida *partida = jogo->partida;
    if (partida == NULL) return;

    bool tutorial = partida_eh_tutorial(partida);
    if (!tutorial && jogo->saveAtual >= 0) {
        Save *save = &jogo->saves[jogo->saveAtual];
        save->tempoJogado += partida->tempoJogado;
        if (venceu) {
            save->faseConcluida[partida->fase - 1] = true;
            if (partida->fase < TOTAL_FASES && save->faseLiberada <= partida->fase) save->faseLiberada = partida->fase + 1;
        }
        gravar_saves(jogo);
    }

    partida_destruir(partida);
    jogo->partida = NULL;
    jogo->estado = tutorial ? ESTADO_MENU : ESTADO_FASES;
}

/* atualizar_menu: troca de tela conforme o botão clicado no menu. */
static void atualizar_menu(Jogo *jogo)
{
    if (ui_botao_clicado(&botoesMenu[BOTAO_JOGAR], jogo->mouse))      jogo->estado = ESTADO_SAVES;
    if (ui_botao_clicado(&botoesMenu[BOTAO_TUTORIAL], jogo->mouse))   iniciar_partida(jogo, FASE_TUTORIAL);
    if (ui_botao_clicado(&botoesMenu[BOTAO_INSTRUCOES], jogo->mouse)) jogo->estado = ESTADO_INSTRUCOES;
    if (ui_botao_clicado(&botoesMenu[BOTAO_SAIR], jogo->mouse))       jogo->estado = ESTADO_SAIR;
}

/* desenhar_menu: cenário, título balançando, subtítulo, botões, versão e o
 * estado da música (tecla M). */
static void desenhar_menu(const Jogo *jogo)
{
    cenario_desenhar_menu(jogo->tempo);
    titulo_desenhar(40 + (int)(sinf(jogo->tempo * 2.0f) * 3.0f), jogo->tempo);
    ui_texto_centro("Cães vs. Gatos Zumbis", LARGURA_VIRTUAL / 2, 104, FONTE_MEDIA, COR_TEXTO);

    for (int i = 0; i < TOTAL_BOTOES_MENU; i++) ui_desenhar_botao(&botoesMenu[i], jogo->mouse);
    ui_texto(VERSAO_JOGO, 6, ALTURA_VIRTUAL - 14, FONTE_PEQUENA, COR_TEXTO);

    const char *som = !jogo->audio.pronto ? "Sem áudio" : (jogo->audio.mudo ? "M: ligar música" : "M: desligar música");
    ui_texto(som, LARGURA_VIRTUAL - 6 - MeasureText(som, FONTE_PEQUENA), ALTURA_VIRTUAL - 14, FONTE_PEQUENA, COR_TEXTO);
}

/* retangulo_save: área do cartão do save i. */
static Rectangle retangulo_save(int i)
{
    return (Rectangle){ 32.0f + i * 200.0f, 60, 176, 220 };
}

/* botoes_do_save: monta os botões do cartão do save i conforme a situação:
 * espaço vazio (Novo Jogo), save em uso (Jogar / Apagar) ou confirmação de
 * exclusão (Sim / Não). Preenche o vetor e retorna quantos botões há. */
static int botoes_do_save(const Jogo *jogo, int i, Botao botoes[2])
{
    Rectangle r = retangulo_save(i);
    if (!jogo->saves[i].usado) {
        botoes[0] = (Botao){ { r.x + 18, r.y + 178, 140, 30 }, "Novo Jogo" };
        return 1;
    }
    bool confirmando = (jogo->saveParaApagar == i);
    botoes[0] = (Botao){ { r.x + 10, r.y + 178, 76, 30 }, confirmando ? "Sim" : "Jogar" };
    botoes[1] = (Botao){ { r.x + 90, r.y + 178, 76, 30 }, confirmando ? "Não" : "Apagar" };
    return 2;
}

/* atualizar_saves: Voltar/ESC retorna ao menu. Em cada cartão: Novo Jogo ou
 * Jogar abrem o mapa de fases do save; Apagar pede confirmação; Sim apaga e
 * Não cancela. Toda mudança é gravada no arquivo na hora. */
static void atualizar_saves(Jogo *jogo)
{
    if (ui_botao_clicado(&botaoVoltar, jogo->mouse) || IsKeyPressed(KEY_ESCAPE)) {
        jogo->saveParaApagar = -1;
        jogo->estado = ESTADO_MENU;
        return;
    }

    for (int i = 0; i < TOTAL_SAVES; i++) {
        Botao botoes[2];
        int total = botoes_do_save(jogo, i, botoes);
        bool primeiro = ui_botao_clicado(&botoes[0], jogo->mouse);
        bool segundo = total > 1 && ui_botao_clicado(&botoes[1], jogo->mouse);
        Save *save = &jogo->saves[i];

        if (jogo->saveParaApagar == i) {
            if (primeiro) {
                save_limpar(save);
                gravar_saves(jogo);
            }
            if (primeiro || segundo) jogo->saveParaApagar = -1;
        } else if (primeiro) {
            if (!save->usado) {
                save_novo(save);
                gravar_saves(jogo);
            }
            jogo->saveAtual = i;
            jogo->saveParaApagar = -1;
            jogo->estado = ESTADO_FASES;
        } else if (segundo) {
            jogo->saveParaApagar = i;
        }
    }
}

/* desenhar_saves: os 3 cartões de save. Vazio mostra "Vazio"; em uso mostra
 * a fase atual, as fases vencidas, os pontos e o tempo jogado. */
static void desenhar_saves(const Jogo *jogo)
{
    cenario_desenhar_menu(jogo->tempo);
    ui_texto_centro("Escolha um Save", LARGURA_VIRTUAL / 2, 24, FONTE_MEDIA, COR_INFECTADO);

    for (int i = 0; i < TOTAL_SAVES; i++) {
        const Save *s = &jogo->saves[i];
        Rectangle r = retangulo_save(i);
        float cx = r.x + r.width / 2;

        ui_desenhar_painel(r);
        ui_texto_centro(TextFormat("SAVE %d", i + 1), cx, r.y + 12, FONTE_MEDIA, COR_PETISCO);
        if (!s->usado) {
            ui_texto_centro("- Vazio -", cx, r.y + 90, FONTE_MEDIA, COR_BLOQUEADO);
        } else {
            ui_texto_centro(TextFormat("Fase %d de %d", s->faseLiberada, TOTAL_FASES), cx, r.y + 50, FONTE_MEDIA, COR_TEXTO);
            ui_texto_centro(TextFormat("Vencidas: %d/%d", save_fases_concluidas(s), TOTAL_FASES), cx, r.y + 90, FONTE_PEQUENA, COR_TEXTO);
            ui_texto_centro(TextFormat("Pontos: %d", s->pontuacao), cx, r.y + 108, FONTE_PEQUENA, COR_TEXTO);
            ui_texto_centro(TextFormat("Tempo: %s", texto_tempo(s->tempoJogado)), cx, r.y + 126, FONTE_PEQUENA, COR_TEXTO);
        }
        if (jogo->saveParaApagar == i) ui_texto_centro("Apagar este save?", cx, r.y + 156, FONTE_PEQUENA, COR_ALERTA);

        Botao botoes[2];
        int total = botoes_do_save(jogo, i, botoes);
        for (int b = 0; b < total; b++) ui_desenhar_botao(&botoes[b], jogo->mouse);
    }
    ui_desenhar_botao(&botaoVoltar, jogo->mouse);
}

/* posicao_fase: centro do círculo da fase f (0 a 4) no mapa de fases. */
static Vector2 posicao_fase(int f)
{
    return (Vector2){ 80.0f + f * 120.0f, 170 };
}

/* atualizar_fases: Voltar/ESC retorna aos saves; clicar numa fase liberada
 * começa a partida dela. */
static void atualizar_fases(Jogo *jogo)
{
    if (ui_botao_clicado(&botaoVoltar, jogo->mouse) || IsKeyPressed(KEY_ESCAPE)) {
        jogo->estado = ESTADO_SAVES;
        return;
    }
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;

    for (int f = 0; f < jogo->saves[jogo->saveAtual].faseLiberada; f++) {
        if (CheckCollisionPointCircle(jogo->mouse, posicao_fase(f), RAIO_FASE)) {
            iniciar_partida(jogo, f + 1);
            return;
        }
    }
}

/* desenhar_fases: as 5 fases do save como círculos ligados por uma trilha:
 * verde = vencida, marrom = liberada, cinza = bloqueada; a 5ª (chefão) tem
 * borda vermelha. Embaixo, os pontos e o tempo do save. */
static void desenhar_fases(const Jogo *jogo)
{
    const Save *s = &jogo->saves[jogo->saveAtual];

    cenario_desenhar_menu(jogo->tempo);
    ui_texto_centro(TextFormat("Save %d - Escolha a Fase", jogo->saveAtual + 1), LARGURA_VIRTUAL / 2, 24, FONTE_MEDIA, COR_INFECTADO);
    ui_desenhar_painel((Rectangle){ 24, 92, 592, 212 });
    DrawLineEx(posicao_fase(0), posicao_fase(TOTAL_FASES - 1), 4, (Color){ 120, 92, 64, 255 });

    for (int f = 0; f < TOTAL_FASES; f++) {
        Vector2 pos = posicao_fase(f);
        bool liberada = f < s->faseLiberada;
        bool chefao = (f == TOTAL_FASES - 1);
        Color fundo = s->faseConcluida[f] ? (Color){ 70, 140, 44, 255 } : (liberada ? COR_BOTAO : (Color){ 60, 56, 64, 255 });

        DrawCircleV(pos, RAIO_FASE + 3, chefao ? COR_SANGUE : COR_BORDA);
        DrawCircleV(pos, RAIO_FASE, fundo);
        ui_texto_centro(TextFormat("%d", f + 1), pos.x, pos.y - 18, FONTE_GRANDE, liberada ? COR_TEXTO : COR_BLOQUEADO);

        const char *rotulo = chefao ? "Chefão" : (liberada ? TextFormat("Fase %d", f + 1) : "Bloqueada");
        ui_texto_centro(rotulo, pos.x, pos.y + RAIO_FASE + 8, FONTE_PEQUENA, liberada ? COR_TEXTO : COR_BLOQUEADO);
    }

    ui_texto_centro(TextFormat("Pontos: %d    Tempo de jogo: %s", s->pontuacao, texto_tempo(s->tempoJogado)),
                    LARGURA_VIRTUAL / 2, 256, FONTE_PEQUENA, COR_TEXTO);
    ui_texto_centro("(Teste: F9 durante a partida vence a fase)", LARGURA_VIRTUAL / 2, 276, FONTE_PEQUENA, COR_BLOQUEADO);
    ui_desenhar_botao(&botaoVoltar, jogo->mouse);
}

/* desenhar_instrucoes: painel com o texto de como jogar e o botão Voltar. */
static void desenhar_instrucoes(const Jogo *jogo)
{
    static const char *linhas[] = {
        "Os Gatos Zumbis avançam pela direita rumo à sua casa. Defenda-a!",
        "",
        "- Clique em uma carta (ou teclas 1 a 6) e depois em um espaço do campo.",
        "- Cada cachorro custa petiscos (ossos). Você começa com 200.",
        "- O Pote de Ração gera petiscos. Clique nos ossos para coletá-los.",
        "- A pá (tecla S) remove um cachorro. O botão direito cancela.",
        "- Sobreviva às 4 hordas de cada fase. Na 5ª fase, enfrente o chefão!",
        "- Se um gato chegar à casa, você perde a fase.",
        "",
        "ESC: pausar     M: música     F11: tela cheia",
    };

    cenario_desenhar_menu(jogo->tempo);
    ui_desenhar_painel((Rectangle){ 50, 30, 540, 264 });
    ui_texto_centro("Como Jogar", LARGURA_VIRTUAL / 2, 44, FONTE_MEDIA, COR_INFECTADO);
    for (int i = 0; i < (int)(sizeof(linhas) / sizeof(linhas[0])); i++) {
        ui_texto(linhas[i], 70, 84 + i * 18, FONTE_PEQUENA, COR_TEXTO);
    }
    ui_desenhar_botao(&botaoVoltar, jogo->mouse);
}

/* atualizar_jogando: depois da derrota, só espera o clique em "Voltar ao
 * Mapa". Senão: ESC/P pausa; F9 vence a fase na hora (atalho de teste até as
 * hordas existirem); ou atualiza a partida, com o tempo do frame limitado a
 * 0,1 s para evitar "saltos" se o jogo travar um instante. */
static void atualizar_jogando(Jogo *jogo)
{
    if (jogo->partida->derrota) {
        if (ui_botao_clicado(&botaoDerrota, jogo->mouse)) encerrar_partida(jogo, false);
        return;
    }
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) jogo->estado = ESTADO_PAUSA;
    else if (IsKeyPressed(KEY_F9) && !partida_eh_tutorial(jogo->partida)) encerrar_partida(jogo, true);
    else partida_atualizar(jogo->partida, jogo->mouse, fminf(GetFrameTime(), 0.1f));
}

/* desenhar_jogando: desenha a partida; se um gato invadiu a casa, escurece a
 * tela e mostra o aviso de derrota com o botão para voltar ao mapa. */
static void desenhar_jogando(const Jogo *jogo)
{
    partida_desenhar(jogo->partida, jogo->mouse);
    if (!jogo->partida->derrota) return;

    DrawRectangle(0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL, Fade(BLACK, 0.6f));
    ui_texto_centro("A CASA FOI INVADIDA!", LARGURA_VIRTUAL / 2, 120, FONTE_GRANDE, COR_ALERTA);
    ui_desenhar_botao(&botaoDerrota, jogo->mouse);
}

/* desenhar_pausa: a partida congelada ao fundo, escurecida, com os botões
 * Continuar e Sair da Partida por cima. */
static void desenhar_pausa(const Jogo *jogo)
{
    partida_desenhar(jogo->partida, (Vector2){ -100, -100 });
    DrawRectangle(0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL, Fade(BLACK, 0.6f));
    ui_texto_centro("PAUSADO", LARGURA_VIRTUAL / 2, 100, FONTE_GRANDE, COR_TEXTO);
    ui_desenhar_botao(&botaoContinuar, jogo->mouse);
    ui_desenhar_botao(&botaoSairPartida, jogo->mouse);
}

/* jogo_criar: aloca o jogo com malloc, cria a tela virtual com filtro POINT
 * (ao ampliar, cada pixel vira um bloco, sem borrar), lê os saves do arquivo
 * e começa a música. Retorna o ponteiro, ou NULL se faltar memória. */
Jogo *jogo_criar(void)
{
    Jogo *jogo = malloc(sizeof(Jogo));
    if (jogo == NULL) return NULL;

    jogo->estado = ESTADO_MENU;
    jogo->mouse = (Vector2){ 0, 0 };
    jogo->tempo = 0.0f;
    jogo->saveAtual = -1;
    jogo->saveParaApagar = -1;
    jogo->partida = NULL;
    jogo->tela = LoadRenderTexture(LARGURA_VIRTUAL, ALTURA_VIRTUAL);
    SetTextureFilter(jogo->tela.texture, TEXTURE_FILTER_POINT);
    save_carregar_todos(jogo->saves);
    audio_iniciar(&jogo->audio);
    return jogo;
}

/* jogo_atualizar: avança o tempo e a música, converte o mouse para a tela
 * virtual (SetMouseOffset/SetMouseScale desfazem a escala e as bordas),
 * trata o F11 e executa a lógica da tela atual. */
void jogo_atualizar(Jogo *jogo)
{
    Rectangle area = area_da_tela();
    SetMouseOffset(-(int)area.x, -(int)area.y);
    SetMouseScale(LARGURA_VIRTUAL / area.width, ALTURA_VIRTUAL / area.height);
    jogo->mouse = GetMousePosition();
    jogo->tempo += GetFrameTime();
    audio_atualizar(&jogo->audio);

    if (IsKeyPressed(KEY_F11)) ToggleBorderlessWindowed();

    switch (jogo->estado) {
        case ESTADO_MENU:    atualizar_menu(jogo);     break;
        case ESTADO_SAVES:   atualizar_saves(jogo);    break;
        case ESTADO_FASES:   atualizar_fases(jogo);    break;
        case ESTADO_JOGANDO: atualizar_jogando(jogo);  break;
        case ESTADO_INSTRUCOES:
            if (ui_botao_clicado(&botaoVoltar, jogo->mouse) || IsKeyPressed(KEY_ESCAPE)) jogo->estado = ESTADO_MENU;
            break;
        case ESTADO_PAUSA:
            if (ui_botao_clicado(&botaoContinuar, jogo->mouse) || IsKeyPressed(KEY_ESCAPE)) jogo->estado = ESTADO_JOGANDO;
            else if (ui_botao_clicado(&botaoSairPartida, jogo->mouse)) encerrar_partida(jogo, false);
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
        case ESTADO_MENU:       desenhar_menu(jogo);                          break;
        case ESTADO_SAVES:      desenhar_saves(jogo);                         break;
        case ESTADO_FASES:      desenhar_fases(jogo);                         break;
        case ESTADO_INSTRUCOES: desenhar_instrucoes(jogo);                    break;
        case ESTADO_JOGANDO:    desenhar_jogando(jogo);                       break;
        case ESTADO_PAUSA:      desenhar_pausa(jogo);                         break;
        case ESTADO_SAIR:                                                     break;
    }
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);
    Rectangle origem = { 0, 0, LARGURA_VIRTUAL, -ALTURA_VIRTUAL };
    DrawTexturePro(jogo->tela.texture, origem, area_da_tela(), (Vector2){ 0, 0 }, 0.0f, WHITE);
    EndDrawing();
}

/* jogo_destruir: se o jogo for fechado no meio de uma partida, salva o tempo
 * jogado; depois libera a música, a tela virtual e o próprio jogo. */
void jogo_destruir(Jogo *jogo)
{
    if (jogo == NULL) return;
    encerrar_partida(jogo, false);
    audio_finalizar(&jogo->audio);
    UnloadRenderTexture(jogo->tela);
    free(jogo);
}
