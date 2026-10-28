/* deck.c - Tela de escolha do deck antes de cada fase. */

#include "deck.h"
#include "cenario.h"
#include "config.h"
#include "ui.h"

static const Color COR_CARTA = { 196, 176, 140, 255 };
static const Color COR_FAIXA = {  90,  64,  48, 255 };

static const Botao botaoVoltar = { { 150, 304, 150, 28 }, "Voltar" };
static const Botao botaoComecar = { { 340, 304, 150, 28 }, "Começar!" };

/* retangulo_opcao: área da carta da raça i (0 a 9) na grade de 2 x 5. */
static Rectangle retangulo_opcao(int i)
{
    return (Rectangle){ 124.0f + (i % 5) * 80.0f, 118.0f + (i / 5) * 80.0f, 72, 72 };
}

/* retangulo_espaco: área do espaço i da linha "Seu deck" (0 = Pote de Ração). */
static Rectangle retangulo_espaco(int i)
{
    return (Rectangle){ 170.0f + i * 50.0f, 48, 46, 52 };
}

/* raca_da_opcao: tipo da raça da carta i da grade (pula o Pote, tipo 0). */
static TipoCachorro raca_da_opcao(int i)
{
    return (TipoCachorro)(i + 1);
}

/* posicao_no_deck: posição da raça no deck escolhido, ou -1 se ela não está nele. */
static int posicao_no_deck(const EscolhaDeck *deck, TipoCachorro tipo)
{
    for (int i = 0; i < deck->total; i++) {
        if (deck->escolhidos[i] == tipo) return i;
    }
    return -1;
}

/* remover_do_deck: tira a raça da posição dada e puxa as seguintes uma casa
 * para trás, para o deck continuar sem buracos. */
static void remover_do_deck(EscolhaDeck *deck, int posicao)
{
    for (int i = posicao; i < deck->total - 1; i++) deck->escolhidos[i] = deck->escolhidos[i + 1];
    deck->total--;
}

/* alternar_raca: tira a raça do deck se ela já estiver nele; senão, a põe
 * no fim (se ainda houver espaço). */
static void alternar_raca(EscolhaDeck *deck, TipoCachorro tipo)
{
    int posicao = posicao_no_deck(deck, tipo);
    if (posicao >= 0)                    remover_do_deck(deck, posicao);
    else if (deck->total < TAMANHO_DECK) deck->escolhidos[deck->total++] = tipo;
}

/* deck_iniciar: abre a tela para a fase pedida. As raças escolhidas da vez
 * anterior continuam marcadas. */
void deck_iniciar(EscolhaDeck *deck, int fase)
{
    deck->fase = fase;
}

/* deck_atualizar: ESC ou Voltar retornam ao mapa; Começar (ou ENTER) só
 * funciona com o deck completo. Um clique numa carta da grade a põe ou tira
 * do deck, e um clique num espaço do deck tira a raça dele. Retorna a ação
 * escolhida pelo jogador. */
AcaoDeck deck_atualizar(EscolhaDeck *deck, Vector2 mouse)
{
    bool completo = (deck->total == TAMANHO_DECK);
    if (IsKeyPressed(KEY_ESCAPE) || ui_botao_clicado(&botaoVoltar, mouse)) return DECK_VOLTAR;
    if (completo && (IsKeyPressed(KEY_ENTER) || ui_botao_clicado(&botaoComecar, mouse))) return DECK_COMECAR;
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return DECK_NADA;

    for (int i = 0; i < TOTAL_RACAS; i++) {
        if (CheckCollisionPointRec(mouse, retangulo_opcao(i))) alternar_raca(deck, raca_da_opcao(i));
    }
    for (int i = 0; i < deck->total; i++) {
        if (CheckCollisionPointRec(mouse, retangulo_espaco(i + 1))) {
            remover_do_deck(deck, i);
            break;
        }
    }
    return DECK_NADA;
}

/* desenhar_espacos: a linha "Seu deck": o Pote de Ração fixo no 1º espaço e
 * os 5 espaços do deck, preenchidos na ordem em que as raças foram escolhidas. */
static void desenhar_espacos(const EscolhaDeck *deck)
{
    ui_texto("Seu deck:", 100, 68, FONTE_PEQUENA, COR_TEXTO);
    for (int i = 0; i <= TAMANHO_DECK; i++) {
        Rectangle r = retangulo_espaco(i);
        bool preenchido = (i == 0 || i <= deck->total);
        TipoCachorro tipo = (i == 0) ? TIPO_POTE_RACAO : deck->escolhidos[i - 1];

        DrawRectangleRec(r, preenchido ? COR_CARTA : Fade(BLACK, 0.4f));
        if (preenchido) cachorro_desenhar(tipo, (Vector2){ r.x + r.width / 2, r.y + 24 }, 13, 0, WHITE);
        DrawRectangleLinesEx(r, 2, COR_BORDA);
    }
}

/* desenhar_opcao: carta de uma raça na grade: o desenho em cima e, na faixa
 * de baixo, o nome e o custo. Já no deck, fica escurecida e com borda verde;
 * com o mouse em cima, a borda fica laranja. */
static void desenhar_opcao(Rectangle r, TipoCachorro tipo, bool noDeck, bool mouseEmCima)
{
    const InfoCachorro *info = cachorro_info(tipo);
    float centroX = r.x + r.width / 2;

    DrawRectangleRec(r, COR_CARTA);
    DrawRectangle(r.x, r.y + 46, r.width, r.height - 46, COR_FAIXA);
    cachorro_desenhar(tipo, (Vector2){ centroX, r.y + 23 }, 16, 0, WHITE);
    ui_texto_centro(info->nome, centroX, r.y + 48, FONTE_PEQUENA, COR_TEXTO);
    ui_texto_centro(TextFormat("%d", info->custo), centroX, r.y + 60, FONTE_PEQUENA, COR_PETISCO);

    if (noDeck) DrawRectangleRec(r, Fade(BLACK, 0.45f));
    DrawRectangleLinesEx(r, 2, noDeck ? COR_INFECTADO : (mouseEmCima ? COR_PETISCO : COR_BORDA));
}

/* desenhar_dica: abaixo da grade, o papel, o custo e a recarga da raça sob
 * o mouse; sem nenhuma, quantas raças já foram escolhidas. */
static void desenhar_dica(const EscolhaDeck *deck, Vector2 mouse)
{
    const char *texto = TextFormat("Escolhidos: %d de %d. Clique nas cartas para montar o deck.", deck->total, TAMANHO_DECK);
    for (int i = 0; i < TOTAL_RACAS; i++) {
        const InfoCachorro *info = cachorro_info(raca_da_opcao(i));
        if (CheckCollisionPointRec(mouse, retangulo_opcao(i))) {
            texto = TextFormat("%s - %s  |  Custo: %d  |  Recarga: %.1f s", info->nome, info->papel, info->custo, info->recarga);
        }
    }
    ui_texto_centro(texto, LARGURA_VIRTUAL / 2, 280, FONTE_PEQUENA, COR_TEXTO);
}

/* deck_desenhar: cenário do menu, painel com o título, a linha "Seu deck",
 * a grade com as 10 raças, a dica e os botões (Começar fica apagado até o
 * deck estar completo). */
void deck_desenhar(const EscolhaDeck *deck, Vector2 mouse, float tempo)
{
    cenario_desenhar_menu(tempo);
    ui_desenhar_painel((Rectangle){ 24, 14, 592, 332 });
    ui_texto_centro(TextFormat("Fase %d - Escolha %d cachorros", deck->fase, TAMANHO_DECK), LARGURA_VIRTUAL / 2, 22, FONTE_MEDIA, COR_INFECTADO);
    desenhar_espacos(deck);

    for (int i = 0; i < TOTAL_RACAS; i++) {
        Rectangle r = retangulo_opcao(i);
        desenhar_opcao(r, raca_da_opcao(i), posicao_no_deck(deck, raca_da_opcao(i)) >= 0, CheckCollisionPointRec(mouse, r));
    }
    desenhar_dica(deck, mouse);

    ui_desenhar_botao(&botaoVoltar, mouse);
    ui_desenhar_botao(&botaoComecar, mouse);
    if (deck->total < TAMANHO_DECK) DrawRectangleRec(botaoComecar.area, Fade(BLACK, 0.5f));
}
