/* game.c - Máquina de estados das telas: menu, saves, fases, instruções,
 * ranking, partida (com a tela de resultado) e pausa. */

#include "game.h"
#include "cenario.h"
#include "config.h"
#include "som.h"
#include "sprite.h"
#include "ui.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAIO_FASE 28

enum { BOTAO_JOGAR, BOTAO_TUTORIAL, BOTAO_INSTRUCOES, BOTAO_RANKING, BOTAO_SAIR, TOTAL_BOTOES_MENU };

static const Botao botoesMenu[TOTAL_BOTOES_MENU] = {
    { { 240, 136, 160, 30 }, "Jogar"      },
    { { 240, 172, 160, 30 }, "Tutorial"   },
    { { 240, 208, 160, 30 }, "Instruções" },
    { { 240, 244, 160, 30 }, "Ranking"    },
    { { 240, 280, 160, 30 }, "Sair"       },
};
static const Botao botaoVoltar = { { 240, 312, 160, 30 }, "Voltar" };
static const Rectangle DESLIZADOR_MUSICA = { 520, 324, 80, 6 };
static const Rectangle DESLIZADOR_EFEITOS = { 520, 342, 80, 6 };
static const Botao botaoContinuar = { { 220, 160, 200, 30 }, "Continuar" };
static const Botao botaoSairPartida = { { 220, 200, 200, 30 }, "Sair da Partida" };

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
 * tutorial) com o deck escolhido, toca o som de início e vai para a tela de
 * jogo. Se faltar memória, fica onde está. */
static void iniciar_partida(Jogo *jogo, int fase)
{
    jogo->partida = partida_criar(fase, jogo->deck.escolhidos);
    if (jogo->partida == NULL) return;
    som_tocar(SOM_INICIAR);
    jogo->estado = ESTADO_JOGANDO;
}

/* abrir_fase: as fases passam antes pela escolha do deck; o tutorial, que
 * tem deck fixo, e a fase do chefão, que usa a esteira, começam direto. */
static void abrir_fase(Jogo *jogo, int fase)
{
    if (fase == FASE_TUTORIAL || fase == FASE_CHEFAO) {
        iniciar_partida(jogo, fase);
        return;
    }
    deck_iniciar(&jogo->deck, fase);
    jogo->estado = ESTADO_DECK;
}

/* atualizar_deck: na escolha do deck, começa a fase ou volta ao mapa. */
static void atualizar_deck(Jogo *jogo)
{
    AcaoDeck acao = deck_atualizar(&jogo->deck, jogo->mouse);
    if (acao == DECK_COMECAR)     iniciar_partida(jogo, jogo->deck.fase);
    else if (acao == DECK_VOLTAR) jogo->estado = ESTADO_FASES;
}

/* lugar_no_ranking: lugar (0 a 9) que a partida terminada ocupa no Top 10,
 * ou -1 se ela não entra (só as vitórias nas fases entram, o tutorial não). */
static int lugar_no_ranking(const Jogo *jogo)
{
    const Partida *partida = jogo->partida;
    if (partida->resultado != PARTIDA_VITORIA || partida_eh_tutorial(partida)) return -1;
    return ranking_posicao(&jogo->ranking, partida_pontuacao(partida).total);
}

/* encerrar_partida: soma o tempo jogado ao save atual e, se a partida foi
 * vencida, marca a fase como concluída, libera a próxima e guarda os pontos
 * se forem o recorde da fase; grava o arquivo. Se a vitória entrar no Top
 * 10, põe no ranking com o nome digitado e grava o ranking. Por fim, destrói
 * a partida e volta para o mapa de fases (ou para o menu, se era o
 * tutorial). */
static void encerrar_partida(Jogo *jogo)
{
    Partida *partida = jogo->partida;
    if (partida == NULL) return;

    bool tutorial = partida_eh_tutorial(partida);
    Pontuacao pontos = partida_pontuacao(partida);
    if (!tutorial && jogo->saveAtual >= 0) {
        Save *save = &jogo->saves[jogo->saveAtual];
        save->tempoJogado += partida->tempoJogado;
        if (partida->resultado == PARTIDA_VITORIA) {
            save->faseConcluida[partida->fase - 1] = true;
            if (partida->fase < TOTAL_FASES && save->faseLiberada <= partida->fase) save->faseLiberada = partida->fase + 1;
            save_registrar_pontos(save, partida->fase, pontos.total);
        }
        gravar_saves(jogo);
    }
    if (lugar_no_ranking(jogo) >= 0) {
        ranking_inserir(&jogo->ranking, jogo->nome, partida->fase, pontos.total, partida->tempoJogado);
        if (!ranking_gravar(&jogo->ranking)) fprintf(stderr, "Aviso: não foi possível gravar %s\n", ARQUIVO_RECORDES);
    }

    partida_destruir(partida);
    jogo->partida = NULL;
    jogo->estado = tutorial ? ESTADO_MENU : ESTADO_FASES;
}

/* atualizar_menu: troca de tela conforme o botão clicado no menu e trata
 * os deslizadores de volume da música e dos efeitos (ao soltar o dos
 * efeitos, toca um som para o jogador ouvir o volume novo). */
static void atualizar_menu(Jogo *jogo)
{
    float musica = jogo->audio.volume, efeitos = som_volume();
    if (ui_deslizador_atualizar(DESLIZADOR_MUSICA, &musica, jogo->mouse)) audio_definir_volume(&jogo->audio, musica);
    if (ui_deslizador_atualizar(DESLIZADOR_EFEITOS, &efeitos, jogo->mouse)) som_definir_volume(efeitos);
    Rectangle folgaEfeitos = { DESLIZADOR_EFEITOS.x - 6, DESLIZADOR_EFEITOS.y - 6, DESLIZADOR_EFEITOS.width + 12, 18 };
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(jogo->mouse, folgaEfeitos)) som_tocar(SOM_PETISCO);

    if (ui_botao_clicado(&botoesMenu[BOTAO_JOGAR], jogo->mouse))      jogo->estado = ESTADO_SAVES;
    if (ui_botao_clicado(&botoesMenu[BOTAO_TUTORIAL], jogo->mouse))   iniciar_partida(jogo, FASE_TUTORIAL);
    if (ui_botao_clicado(&botoesMenu[BOTAO_INSTRUCOES], jogo->mouse)) jogo->estado = ESTADO_INSTRUCOES;
    if (ui_botao_clicado(&botoesMenu[BOTAO_RANKING], jogo->mouse))    jogo->estado = ESTADO_RANKING;
    if (ui_botao_clicado(&botoesMenu[BOTAO_SAIR], jogo->mouse))       jogo->estado = ESTADO_SAIR;
}

/* desenhar_menu: cenário, título balançando, subtítulo, botões, versão e,
 * no canto, os deslizadores de volume (ou "Sem áudio"). */
static void desenhar_menu(const Jogo *jogo)
{
    cenario_desenhar_menu(jogo->tempo);
    titulo_desenhar(40 + (int)(sinf(jogo->tempo * 2.0f) * 3.0f), jogo->tempo);
    ui_texto_centro("Cães vs. Gatos Zumbis", LARGURA_VIRTUAL / 2, 104, FONTE_MEDIA, COR_TEXTO);

    for (int i = 0; i < TOTAL_BOTOES_MENU; i++) ui_desenhar_botao(&botoesMenu[i], jogo->mouse);
    ui_texto(VERSAO_JOGO, 6, ALTURA_VIRTUAL - 14, FONTE_PEQUENA, COR_TEXTO);

    if (!jogo->audio.pronto) {
        ui_texto("Sem áudio", LARGURA_VIRTUAL - 6 - MeasureText("Sem áudio", FONTE_PEQUENA), ALTURA_VIRTUAL - 14, FONTE_PEQUENA, COR_TEXTO);
        return;
    }
    ui_desenhar_deslizador(DESLIZADOR_MUSICA, jogo->audio.mudo ? 0.0f : jogo->audio.volume, jogo->audio.mudo ? "Mudo (M)" : "Música");
    ui_desenhar_deslizador(DESLIZADOR_EFEITOS, som_volume(), "Efeitos");
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
 * abre a escolha do deck dela. */
static void atualizar_fases(Jogo *jogo)
{
    if (ui_botao_clicado(&botaoVoltar, jogo->mouse) || IsKeyPressed(KEY_ESCAPE)) {
        jogo->estado = ESTADO_SAVES;
        return;
    }
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;

    for (int f = 0; f < jogo->saves[jogo->saveAtual].faseLiberada; f++) {
        if (CheckCollisionPointCircle(jogo->mouse, posicao_fase(f), RAIO_FASE)) {
            som_tocar(SOM_CLIQUE);
            abrir_fase(jogo, f + 1);
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
    ui_texto_centro("(Teste: \\ vence a fase, ] dá +100 petiscos)", LARGURA_VIRTUAL / 2, 276, FONTE_PEQUENA, COR_BLOQUEADO);
    ui_desenhar_botao(&botaoVoltar, jogo->mouse);
}

/* desenhar_ranking: painel com o Top 10 (lugar, nome, fase, pontos e
 * tempo de cada recorde) e o botão Voltar. */
static void desenhar_ranking(const Jogo *jogo)
{
    cenario_desenhar_menu(jogo->tempo);
    ui_desenhar_painel((Rectangle){ 100, 30, 440, 264 });
    ui_texto_centro("Top 10 - Melhores Vitórias", LARGURA_VIRTUAL / 2, 44, FONTE_MEDIA, COR_INFECTADO);
    ranking_desenhar(&jogo->ranking, (Rectangle){ 145, 76, 350, 210 });
    ui_desenhar_botao(&botaoVoltar, jogo->mouse);
}

/* desenhar_instrucoes: painel com o texto de como jogar e o botão Voltar. */
static void desenhar_instrucoes(const Jogo *jogo)
{
    static const char *linhas[] = {
        "Os Gatos Zumbis avançam pela direita rumo à sua casa. Defenda-a!",
        "",
        "- Antes de cada fase, escolha 5 cachorros para o seu deck.",
        "- Clique em uma carta (ou teclas 1 a 6) e depois em um espaço do campo.",
        "- Cada cachorro custa petiscos (ossos). Você começa com 200.",
        "- O Pote de Ração gera petiscos. Clique nos ossos para coletá-los.",
        "- A pá (tecla S) remove um cachorro. O botão direito cancela.",
        "- A luva (tecla L) muda um cachorro de lugar: só 2 vezes por fase.",
        "- O botão 2x (tecla V) acelera o jogo.",
        "- Sobreviva às 4 hordas de cada fase. Na 5ª fase, enfrente o chefão:",
        "  os cachorros chegam de graça pela esteira, à esquerda.",
        "- Se um gato chegar à casa, você perde a fase.",
        "",
        "ESC: pausar     M: música     F11: tela cheia",
    };

    cenario_desenhar_menu(jogo->tempo);
    ui_desenhar_painel((Rectangle){ 50, 30, 540, 264 });
    ui_texto_centro("Como Jogar", LARGURA_VIRTUAL / 2, 44, FONTE_MEDIA, COR_INFECTADO);
    for (int i = 0; i < (int)(sizeof(linhas) / sizeof(linhas[0])); i++) {
        ui_texto(linhas[i], 70, 72 + i * 16, FONTE_PEQUENA, COR_TEXTO);
    }
    ui_desenhar_botao(&botaoVoltar, jogo->mouse);
}

/* tem_proxima_fase: true se, depois desta partida, o 1º botão leva para a
 * fase seguinte (vitória numa fase que não é a última nem o tutorial). */
static bool tem_proxima_fase(const Partida *partida)
{
    return partida->resultado == PARTIDA_VITORIA && !partida_eh_tutorial(partida) && partida->fase < TOTAL_FASES;
}

/* botao_fim_partida: monta o 1º botão da tela de fim de partida: "Próxima
 * Fase", "Jogar de Novo" (vitória na última fase ou no tutorial) ou "Tentar
 * de Novo" (derrota). */
static Botao botao_fim_partida(const Partida *partida)
{
    const char *texto = tem_proxima_fase(partida) ? "Próxima Fase" : "Tentar de Novo";
    if (partida->resultado == PARTIDA_VITORIA && !tem_proxima_fase(partida)) texto = "Jogar de Novo";
    return (Botao){ { 220, 250, 200, 30 }, texto };
}

/* botao_voltar_fim: o 2º botão da tela de fim de partida, que volta ao mapa
 * de fases (ou ao menu, se era o tutorial). */
static Botao botao_voltar_fim(const Partida *partida)
{
    return (Botao){ { 220, 288, 200, 30 }, partida_eh_tutorial(partida) ? "Voltar ao Menu" : "Voltar ao Mapa" };
}

/* digitar_nome: lê as letras digitadas (caracteres comuns, até
 * TAMANHO_NOME) e o Backspace, que apaga a última. */
static void digitar_nome(Jogo *jogo)
{
    int tamanho = (int)strlen(jogo->nome);
    for (int c = GetCharPressed(); c != 0; c = GetCharPressed()) {
        if (c < 32 || c > 126 || tamanho >= TAMANHO_NOME) continue;
        jogo->nome[tamanho++] = (char)c;
        jogo->nome[tamanho] = '\0';
    }
    if (IsKeyPressed(KEY_BACKSPACE) && tamanho > 0) jogo->nome[tamanho - 1] = '\0';
}

/* atualizar_fim_partida: na tela de vitória/derrota, se a vitória entrar no
 * Top 10, o jogador digita o nome. O 1º botão encerra a partida (salvando o
 * resultado e o recorde) e abre a próxima fase (ou a mesma de novo),
 * passando pela escolha do deck; o 2º só encerra. */
static void atualizar_fim_partida(Jogo *jogo)
{
    Botao principal = botao_fim_partida(jogo->partida);
    Botao voltar = botao_voltar_fim(jogo->partida);
    int fase = tem_proxima_fase(jogo->partida) ? jogo->partida->fase + 1 : jogo->partida->fase;

    if (lugar_no_ranking(jogo) >= 0) digitar_nome(jogo);

    if (ui_botao_clicado(&principal, jogo->mouse)) {
        encerrar_partida(jogo);
        abrir_fase(jogo, fase);
    } else if (ui_botao_clicado(&voltar, jogo->mouse)) {
        encerrar_partida(jogo);
    }
}

/* atualizar_jogando: com o resultado decidido, trata a tela de vitória ou
 * derrota. Senão: ESC/P pausa; ou atualiza a partida, com o tempo do frame
 * limitado a 0,1 s para evitar "saltos" se o jogo travar um instante. */
static void atualizar_jogando(Jogo *jogo)
{
    if (jogo->partida->resultado != PARTIDA_EM_ANDAMENTO) atualizar_fim_partida(jogo);
    else if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) jogo->estado = ESTADO_PAUSA;
    else partida_atualizar(jogo->partida, jogo->mouse, fminf(GetFrameTime(), 0.1f));
}

/* desenhar_linha_pontos: uma linha da conta de pontos: o texto à esquerda
 * e o valor ("+N") alinhado à direita. */
static void desenhar_linha_pontos(const char *texto, int valor, int y)
{
    const char *numero = TextFormat("+%d", valor);
    ui_texto(texto, 200, y, FONTE_PEQUENA, COR_TEXTO);
    ui_texto(numero, 440 - MeasureText(numero, FONTE_PEQUENA), y, FONTE_PEQUENA, COR_TEXTO);
}

/* desenhar_resultado: abaixo do título, a conta de pontos da partida: gatos
 * derrotados, bônus de vitória e de tempo e o total. Se a vitória entrar no
 * Top 10, mostra o lugar e a caixa com o nome sendo digitado (com um cursor
 * piscando). */
static void desenhar_resultado(const Jogo *jogo)
{
    const Partida *partida = jogo->partida;
    Pontuacao p = partida_pontuacao(partida);
    int lugar = lugar_no_ranking(jogo);

    desenhar_linha_pontos(TextFormat("Gatos derrotados: %d", p.gatos), p.pontosGatos, 112);
    desenhar_linha_pontos("Bônus de vitória", p.vitoria, 128);
    desenhar_linha_pontos("Bônus de tempo", p.tempo, 144);
    ui_texto_centro(TextFormat("TOTAL: %d pontos", p.total), LARGURA_VIRTUAL / 2, 164, FONTE_MEDIA, COR_PETISCO);
    if (lugar < 0) return;

    Rectangle caixa = { 240, 212, 160, 20 };
    bool cursor = fmodf(jogo->tempo, 1.0f) < 0.5f;
    ui_texto_centro(TextFormat("Novo recorde: %dº lugar no Top 10! Digite seu nome:", lugar + 1), LARGURA_VIRTUAL / 2, 194, FONTE_PEQUENA, COR_INFECTADO);
    DrawRectangleRec(caixa, Fade(BLACK, 0.8f));
    DrawRectangleLinesEx(caixa, 1, COR_INFECTADO);
    ui_texto_centro(TextFormat("%s%s", jogo->nome, cursor ? "_" : " "), LARGURA_VIRTUAL / 2, caixa.y + 5, FONTE_PEQUENA, COR_TEXTO);
}

/* desenhar_jogando: desenha a partida; com o resultado decidido, escurece a
 * tela e mostra por cima o título (vitória em verde, derrota em vermelho),
 * o tempo da partida, a conta de pontos (fora do tutorial) e os dois botões. */
static void desenhar_jogando(const Jogo *jogo)
{
    const Partida *partida = jogo->partida;
    partida_desenhar(partida, jogo->mouse);
    if (partida->resultado == PARTIDA_EM_ANDAMENTO) return;

    bool venceu = (partida->resultado == PARTIDA_VITORIA);
    const char *titulo = !venceu ? "A CASA FOI INVADIDA!" : (partida_eh_tutorial(partida) ? "TUTORIAL CONCLUÍDO!" : "FASE CONCLUÍDA!");
    if (venceu && partida->chefao != NULL) titulo = "O REI FOI DERROTADO!";
    Botao principal = botao_fim_partida(partida);
    Botao voltar = botao_voltar_fim(partida);

    DrawRectangle(0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL, Fade(BLACK, 0.6f));
    ui_texto_centro(titulo, LARGURA_VIRTUAL / 2, 36, FONTE_GRANDE, venceu ? COR_INFECTADO : COR_ALERTA);
    ui_texto_centro(TextFormat("Tempo: %s", texto_tempo(partida->tempoJogado)), LARGURA_VIRTUAL / 2, 84, FONTE_MEDIA, COR_TEXTO);
    if (!partida_eh_tutorial(partida)) desenhar_resultado(jogo);
    ui_desenhar_botao(&principal, jogo->mouse);
    ui_desenhar_botao(&voltar, jogo->mouse);
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
 * (ao ampliar, cada pixel vira um bloco, sem borrar), carrega os sprites, lê
 * os saves do arquivo, começa a música e carrega os efeitos sonoros. Retorna
 * o ponteiro, ou NULL se faltar memória. */
Jogo *jogo_criar(void)
{
    Jogo *jogo = malloc(sizeof(Jogo));
    if (jogo == NULL) return NULL;

    jogo->estado = ESTADO_MENU;
    jogo->mouse = (Vector2){ 0, 0 };
    jogo->tempo = 0.0f;
    jogo->saveAtual = -1;
    jogo->saveParaApagar = -1;
    jogo->deck = (EscolhaDeck){ 0 };
    jogo->nome[0] = '\0';
    jogo->partida = NULL;
    jogo->tela = LoadRenderTexture(LARGURA_VIRTUAL, ALTURA_VIRTUAL);
    SetTextureFilter(jogo->tela.texture, TEXTURE_FILTER_POINT);
    sprite_carregar_todos();
    save_carregar_todos(jogo->saves);
    ranking_carregar(&jogo->ranking);
    audio_iniciar(&jogo->audio);
    som_carregar_todos();
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
        case ESTADO_DECK:    atualizar_deck(jogo);     break;
        case ESTADO_JOGANDO: atualizar_jogando(jogo);  break;
        case ESTADO_INSTRUCOES:
        case ESTADO_RANKING:
            if (ui_botao_clicado(&botaoVoltar, jogo->mouse) || IsKeyPressed(KEY_ESCAPE)) jogo->estado = ESTADO_MENU;
            break;
        case ESTADO_PAUSA:
            if (ui_botao_clicado(&botaoContinuar, jogo->mouse) || IsKeyPressed(KEY_ESCAPE)) jogo->estado = ESTADO_JOGANDO;
            else if (ui_botao_clicado(&botaoSairPartida, jogo->mouse)) encerrar_partida(jogo);
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
        case ESTADO_DECK:       deck_desenhar(&jogo->deck, jogo->mouse, jogo->tempo); break;
        case ESTADO_INSTRUCOES: desenhar_instrucoes(jogo);                    break;
        case ESTADO_RANKING:    desenhar_ranking(jogo);                       break;
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

/* jogo_destruir: se o jogo for fechado durante uma partida, salva o tempo
 * jogado (e a vitória, se já estava na tela de vitória); depois libera os
 * efeitos sonoros, a música, os sprites, a tela virtual e o próprio jogo. */
void jogo_destruir(Jogo *jogo)
{
    if (jogo == NULL) return;
    encerrar_partida(jogo);
    som_descarregar_todos();
    audio_finalizar(&jogo->audio);
    sprite_descarregar_todos();
    UnloadRenderTexture(jogo->tela);
    free(jogo);
}
