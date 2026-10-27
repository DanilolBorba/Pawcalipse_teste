/* partida.c - Uma partida: cartas, petiscos, colocação de cachorros, pá,
 * combate contra os gatos e tutorial. */

#include "partida.h"
#include "cenario.h"
#include "config.h"
#include "ui.h"

#include <stdlib.h>

#define TAMANHO(v)      ((int)(sizeof(v) / sizeof((v)[0])))
#define INTERVALO_CEU   8.0f   // segundos mínimos entre petiscos do céu
#define DURACAO_AVISO   1.6f
#define PRIMEIRO_GATO   20.0f  // segundos até o 1º gato (provisório, até as hordas)
#define INTERVALO_GATO  8.0f   // segundos mínimos entre gatos (provisório)

static const Rectangle RET_CONTADOR = { 8, 8, 56, 52 };
static const Rectangle RET_PA = { 380, 8, 46, 52 };

static const TipoCachorro deckTutorial[] = { TIPO_POTE_RACAO, TIPO_BEAGLE, TIPO_BULLDOG };
static const TipoCachorro deckProvisorio[] = {
    TIPO_POTE_RACAO, TIPO_BEAGLE, TIPO_BULLDOG, TIPO_DALMATA, TIPO_HUSKY, TIPO_CHIHUAHUA,
};

enum { TUTORIAL_COLETAR, TUTORIAL_POTE, TUTORIAL_CACHORRO, TUTORIAL_PA, TUTORIAL_FIM };

static const char *textosTutorial[] = {
    "Clique no osso que está caindo para coletar petiscos.",
    "Compre um Pote de Ração: clique na carta e depois no campo.",
    "O pote gera petiscos sozinho! Agora coloque um Beagle.",
    "Use a pá (ou tecla S) para remover um cachorro do campo.",
    "Muito bem! Nas fases, os gatos zumbis vão atacar. ESC para sair.",
};

/* retangulo_carta: área da carta de índice i na barra superior. */
static Rectangle retangulo_carta(int i)
{
    return (Rectangle){ 70.0f + i * 50.0f, 8, 46, 52 };
}

/* avisar: mostra uma mensagem temporária abaixo da barra superior. */
static void avisar(Partida *partida, const char *texto)
{
    partida->aviso = texto;
    partida->timerAviso = DURACAO_AVISO;
}

/* avancar_tutorial: se a partida for o tutorial e o jogador acabou de cumprir
 * o passo atual, passa para a próxima instrução. */
static void avancar_tutorial(Partida *partida, int passoCumprido)
{
    if (partida->fase == FASE_TUTORIAL && partida->passoTutorial == passoCumprido) partida->passoTutorial++;
}

/* partida_criar: aloca a partida com malloc, zera tudo e prepara o campo
 * vazio, as listas de petiscos, gatos e bolinhas vazias, o saldo inicial e
 * o deck (fixo por enquanto: um para o tutorial e outro para as fases).
 * Retorna NULL se faltar memória. */
Partida *partida_criar(int fase)
{
    Partida *partida = malloc(sizeof(Partida));
    if (partida == NULL) return NULL;

    *partida = (Partida){ 0 };
    mapa_iniciar(&partida->mapa);
    partida->petiscos = NULL;
    partida->gatos = NULL;
    partida->projeteis = NULL;
    partida->fase = fase;
    partida->saldo = PETISCOS_INICIAIS;
    partida->cartaSelecionada = -1;
    partida->timerPetiscoCeu = (fase == FASE_TUTORIAL) ? 1.0f : 5.0f;
    partida->timerGato = PRIMEIRO_GATO;

    const TipoCachorro *deck = (fase == FASE_TUTORIAL) ? deckTutorial : deckProvisorio;
    partida->totalCartas = (fase == FASE_TUTORIAL) ? TAMANHO(deckTutorial) : TAMANHO(deckProvisorio);
    for (int i = 0; i < partida->totalCartas; i++) partida->cartas[i].tipo = deck[i];
    return partida;
}

/* partida_destruir: libera os cachorros do campo, as listas de petiscos,
 * gatos e bolinhas e a própria partida. */
void partida_destruir(Partida *partida)
{
    if (partida == NULL) return;
    mapa_limpar(&partida->mapa);
    petisco_liberar_lista(&partida->petiscos);
    gato_liberar_lista(&partida->gatos);
    projetil_liberar_lista(&partida->projeteis);
    free(partida);
}

/* partida_eh_tutorial: retorna true se a partida for o tutorial. */
bool partida_eh_tutorial(const Partida *partida)
{
    return partida->fase == FASE_TUTORIAL;
}

/* soltar_petisco_do_ceu: de tempos em tempos (8 a 12 s) solta um petisco do
 * céu num ponto aleatório do campo. */
static void soltar_petisco_do_ceu(Partida *partida, float dt)
{
    partida->timerPetiscoCeu -= dt;
    if (partida->timerPetiscoCeu > 0.0f) return;

    float x = MAPA_X + 24 + rand() % (COLUNAS_MAPA * TAMANHO_CELULA - 48);
    float alvo = MAPA_Y + 24 + rand() % (LINHAS_MAPA * TAMANHO_CELULA - 40);
    petisco_adicionar(&partida->petiscos, (Vector2){ x, -10 }, alvo);
    partida->timerPetiscoCeu = INTERVALO_CEU + rand() % 5;
}

/* agir_cachorro: conta o tempo até a próxima ação do cachorro e, quando ele
 * chega a zero, age conforme o tipo: o Pote de Ração solta um petisco e o
 * Beagle joga uma bolinha, se houver um gato na linha à frente dele (senão
 * fica esperando, pronto para atirar). */
static void agir_cachorro(Partida *partida, Cachorro *cachorro, float dt)
{
    const InfoCachorro *info = cachorro_info(cachorro->tipo);
    Vector2 centro = mapa_centro_celula(cachorro->linha, cachorro->coluna);

    cachorro->timer -= dt;
    if (cachorro->timer > 0.0f) return;

    switch (cachorro->tipo) {
        case TIPO_POTE_RACAO:
            petisco_adicionar(&partida->petiscos, (Vector2){ centro.x + rand() % 21 - 10, centro.y - 12 }, centro.y + 18);
            cachorro->timer = POTE_INTERVALO_PRODUCAO;
            break;
        case TIPO_BEAGLE:
            if (!gato_na_mira(partida->gatos, cachorro->linha, centro.x)) break;
            projetil_adicionar(&partida->projeteis, (Vector2){ centro.x + 16, centro.y - 4 }, cachorro->linha, info->dano);
            cachorro->timer = info->cadencia;
            break;
        default:
            break;
    }
}

/* agir_cachorros: percorre a matriz do mapa fazendo cada cachorro agir. */
static void agir_cachorros(Partida *partida, float dt)
{
    for (int l = 0; l < LINHAS_MAPA; l++) {
        for (int c = 0; c < COLUNAS_MAPA; c++) {
            if (partida->mapa.grade[l][c] != NULL) agir_cachorro(partida, partida->mapa.grade[l][c], dt);
        }
    }
}

/* gerar_gatos_provisorio: PROVISÓRIO até as hordas (Etapa 5). A cada 8 a 12
 * segundos manda um gato por uma linha aleatória. Depois de 60 s metade
 * deles usa cone; depois de 120 s, 20% usam panela. */
static void gerar_gatos_provisorio(Partida *partida, float dt)
{
    partida->timerGato -= dt;
    if (partida->timerGato > 0.0f) return;

    int sorteio = rand() % 100;
    TipoGato tipo = GATO_COMUM;
    if (partida->tempoJogado > 120.0f && sorteio < 20)     tipo = GATO_PANELA;
    else if (partida->tempoJogado > 60.0f && sorteio < 50) tipo = GATO_CONE;

    gato_adicionar(&partida->gatos, tipo, rand() % LINHAS_MAPA);
    partida->timerGato = INTERVALO_GATO + rand() % 5;
}

/* selecionar_carta: marca a carta i para ser colocada no campo. Clicar de
 * novo na mesma carta desmarca. Avisa se ela estiver recarregando ou se não
 * houver petiscos suficientes. */
static void selecionar_carta(Partida *partida, int i)
{
    const Carta *carta = &partida->cartas[i];

    partida->paSelecionada = false;
    if (partida->cartaSelecionada == i)                          partida->cartaSelecionada = -1;
    else if (carta->recarga > 0.0f)                              avisar(partida, "Carta recarregando!");
    else if (partida->saldo < cachorro_info(carta->tipo)->custo) avisar(partida, "Petiscos insuficientes!");
    else                                                         partida->cartaSelecionada = i;
}

/* colocar_cachorro: cria o cachorro da carta selecionada (malloc) e o guarda
 * na célula, se ela estiver livre. Desconta o custo, inicia a recarga da
 * carta e avança o tutorial. */
static void colocar_cachorro(Partida *partida, int linha, int coluna)
{
    if (partida->mapa.grade[linha][coluna] != NULL) {
        avisar(partida, "Esse espaço já está ocupado!");
        return;
    }

    Carta *carta = &partida->cartas[partida->cartaSelecionada];
    const InfoCachorro *info = cachorro_info(carta->tipo);
    Cachorro *novo = cachorro_criar(carta->tipo, linha, coluna);
    if (novo == NULL) {
        avisar(partida, "Sem memória para um novo cachorro!");
        return;
    }

    mapa_colocar(&partida->mapa, novo);
    partida->saldo -= info->custo;
    carta->recarga = info->recarga;
    partida->cartaSelecionada = -1;
    avancar_tutorial(partida, (carta->tipo == TIPO_POTE_RACAO) ? TUTORIAL_POTE : TUTORIAL_CACHORRO);
}

/* tratar_entrada: lê o teclado (1 a 6 escolhem cartas, S pega a pá, botão
 * direito cancela) e o clique esquerdo, nesta ordem de prioridade: coletar
 * petisco, escolher carta, pegar a pá e, por fim, agir no campo (colocar o
 * cachorro selecionado ou remover com a pá). */
static void tratar_entrada(Partida *partida, Vector2 mouse)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        partida->cartaSelecionada = -1;
        partida->paSelecionada = false;
    }
    for (int i = 0; i < partida->totalCartas; i++) {
        if (IsKeyPressed(KEY_ONE + i)) selecionar_carta(partida, i);
    }
    bool clicou = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (IsKeyPressed(KEY_S) || (clicou && CheckCollisionPointRec(mouse, RET_PA))) {
        partida->paSelecionada = !partida->paSelecionada;
        partida->cartaSelecionada = -1;
        return;
    }
    if (!clicou) return;

    int valor = petisco_coletar(&partida->petiscos, mouse);
    if (valor > 0) {
        partida->saldo += valor;
        avancar_tutorial(partida, TUTORIAL_COLETAR);
        return;
    }
    for (int i = 0; i < partida->totalCartas; i++) {
        if (CheckCollisionPointRec(mouse, retangulo_carta(i))) {
            selecionar_carta(partida, i);
            return;
        }
    }

    int linha, coluna;
    if (!mapa_celula_no_ponto(mouse, &linha, &coluna)) return;
    if (partida->paSelecionada) {
        if (partida->mapa.grade[linha][coluna] != NULL) avancar_tutorial(partida, TUTORIAL_PA);
        mapa_remover(&partida->mapa, linha, coluna);
        partida->paSelecionada = false;
    } else if (partida->cartaSelecionada >= 0) {
        colocar_cachorro(partida, linha, coluna);
    }
}

/* partida_atualizar: avança o tempo da partida, o aviso e a recarga das
 * cartas; faz os cachorros agirem, manda gatos (fora do tutorial), move as
 * bolinhas e os gatos, gera e move os petiscos e trata a entrada do jogador.
 * Se um gato invadir a casa, marca a derrota e a partida congela. */
void partida_atualizar(Partida *partida, Vector2 mouse, float dt)
{
    if (partida->derrota) return;

    partida->tempoJogado += dt;
    if (partida->timerAviso > 0.0f) partida->timerAviso -= dt;
    for (int i = 0; i < partida->totalCartas; i++) {
        if (partida->cartas[i].recarga > 0.0f) partida->cartas[i].recarga -= dt;
    }

    agir_cachorros(partida, dt);
    if (!partida_eh_tutorial(partida)) gerar_gatos_provisorio(partida, dt);
    projetil_atualizar_lista(&partida->projeteis, partida->gatos, dt);
    if (gato_atualizar_lista(&partida->gatos, &partida->mapa, dt)) partida->derrota = true;

    soltar_petisco_do_ceu(partida, dt);
    petisco_atualizar_lista(&partida->petiscos, dt);
    tratar_entrada(partida, mouse);
}

/* desenhar_carta: desenha a carta i: fundo, forma do cachorro, custo (em
 * vermelho se não der para comprar), sombra de recarga que encolhe com o
 * tempo e borda verde se estiver selecionada. */
static void desenhar_carta(const Partida *partida, int i)
{
    const Carta *carta = &partida->cartas[i];
    const InfoCachorro *info = cachorro_info(carta->tipo);
    Rectangle r = retangulo_carta(i);
    bool podeComprar = partida->saldo >= info->custo;

    DrawRectangleRec(r, (Color){ 196, 176, 140, 255 });
    DrawRectangle(r.x, r.y + 38, r.width, 14, (Color){ 90, 64, 48, 255 });
    cachorro_desenhar(carta->tipo, (Vector2){ r.x + r.width / 2, r.y + 20 }, 13, WHITE);
    ui_texto_centro(TextFormat("%d", info->custo), r.x + r.width / 2, r.y + 40, FONTE_PEQUENA, podeComprar ? COR_TEXTO : COR_ALERTA);

    if (carta->recarga > 0.0f) DrawRectangle(r.x, r.y, r.width, r.height * carta->recarga / info->recarga, Fade(BLACK, 0.55f));
    if (!podeComprar) DrawRectangleRec(r, Fade(BLACK, 0.25f));
    DrawRectangleLinesEx(r, 2, (partida->cartaSelecionada == i) ? COR_INFECTADO : COR_BORDA);
}

/* desenhar_hud: barra superior com o contador de petiscos, as cartas, o
 * botão da pá e, à direita, a fase, o tempo e o nome da carta sob o mouse. */
static void desenhar_hud(const Partida *partida, Vector2 mouse)
{
    ui_desenhar_painel((Rectangle){ 4, 4, 632, 60 });
    DrawRectangleRec(RET_CONTADOR, (Color){ 52, 36, 30, 255 });
    ui_desenhar_osso((Vector2){ 36, 24 }, 1.2f, COR_OSSO);
    ui_texto_centro(TextFormat("%d", partida->saldo), 36, 38, FONTE_MEDIA, COR_TEXTO);

    for (int i = 0; i < partida->totalCartas; i++) desenhar_carta(partida, i);

    DrawRectangleRec(RET_PA, partida->paSelecionada ? (Color){ 120, 150, 90, 255 } : (Color){ 110, 100, 96, 255 });
    DrawRectangleLinesEx(RET_PA, 2, partida->paSelecionada ? COR_INFECTADO : COR_BORDA);
    ui_texto_centro("Pá", RET_PA.x + 23, 18, FONTE_MEDIA, COR_TEXTO);
    ui_texto_centro("(S)", RET_PA.x + 23, 42, FONTE_PEQUENA, COR_TEXTO);

    int s = (int)partida->tempoJogado;
    ui_texto(partida_eh_tutorial(partida) ? "TUTORIAL" : TextFormat("FASE %d", partida->fase), 440, 12, FONTE_MEDIA, COR_INFECTADO);
    ui_texto(TextFormat("Tempo: %02d:%02d", s / 60, s % 60), 440, 36, FONTE_PEQUENA, COR_TEXTO);

    const char *dica = "ESC: pausa";
    for (int i = 0; i < partida->totalCartas; i++) {
        const InfoCachorro *info = cachorro_info(partida->cartas[i].tipo);
        if (CheckCollisionPointRec(mouse, retangulo_carta(i))) dica = TextFormat("%s - %s", info->nome, info->papel);
    }
    ui_texto(dica, 440, 48, FONTE_PEQUENA, COR_BLOQUEADO);
}

/* desenhar_previa: destaca a célula sob o mouse. Com uma carta selecionada,
 * mostra o cachorro transparente onde ele será colocado; com carta ou pá,
 * pinta de vermelho uma célula ocupada (onde não dá para colocar ou de onde
 * o cachorro será removido). */
static void desenhar_previa(const Partida *partida, Vector2 mouse)
{
    int linha, coluna;
    if (!mapa_celula_no_ponto(mouse, &linha, &coluna)) return;

    Rectangle r = mapa_retangulo_celula(linha, coluna);
    bool ocupada = partida->mapa.grade[linha][coluna] != NULL;
    bool selecionando = partida->paSelecionada || partida->cartaSelecionada >= 0;

    if (partida->cartaSelecionada >= 0 && !ocupada) {
        cachorro_desenhar(partida->cartas[partida->cartaSelecionada].tipo, mapa_centro_celula(linha, coluna), 22, Fade(WHITE, 0.5f));
    }
    if (selecionando && ocupada) DrawRectangleRec(r, Fade(COR_ALERTA, 0.35f));
    DrawRectangleLinesEx(r, 2, selecionando ? COR_INFECTADO : Fade(WHITE, 0.25f));
}

/* desenhar_mensagem: mostra entre a barra e o campo o aviso temporário (em
 * vermelho) ou, no tutorial, a instrução do passo atual com um contorno
 * laranja em volta do que deve ser clicado. */
static void desenhar_mensagem(const Partida *partida)
{
    const char *texto = NULL;
    Color cor = COR_PETISCO;
    if (partida->timerAviso > 0.0f) {
        texto = partida->aviso;
        cor = COR_ALERTA;
    } else if (partida_eh_tutorial(partida)) {
        texto = textosTutorial[partida->passoTutorial];
    }
    if (texto == NULL) return;

    float largura = MeasureText(texto, FONTE_PEQUENA) + 16;
    Rectangle faixa = { (LARGURA_VIRTUAL - largura) / 2, 70, largura, 18 };
    DrawRectangleRec(faixa, Fade(BLACK, 0.75f));
    DrawRectangleLinesEx(faixa, 1, cor);
    ui_texto_centro(texto, LARGURA_VIRTUAL / 2, 74, FONTE_PEQUENA, cor);

    int passo = partida->passoTutorial;
    if (partida_eh_tutorial(partida) && partida->timerAviso <= 0.0f && passo >= TUTORIAL_POTE && passo <= TUTORIAL_PA) {
        Rectangle alvos[] = { { 0 }, retangulo_carta(0), retangulo_carta(1), RET_PA };
        DrawRectangleLinesEx(alvos[passo], 3, COR_PETISCO);
    }
}

/* partida_desenhar: desenha, de trás para a frente, o céu e os prédios atrás
 * da barra, o campo com os cachorros, os gatos, as bolinhas, a prévia sob o
 * mouse, a barra superior, a faixa de mensagens e os petiscos (por cima de
 * tudo). */
void partida_desenhar(const Partida *partida, Vector2 mouse)
{
    cenario_desenhar_ceu(MAPA_Y, partida->tempoJogado);
    cenario_desenhar_skyline_distante(MAPA_Y);
    mapa_desenhar(&partida->mapa);
    gato_desenhar_lista(partida->gatos);
    projetil_desenhar_lista(partida->projeteis);
    desenhar_previa(partida, mouse);
    desenhar_hud(partida, mouse);
    desenhar_mensagem(partida);
    petisco_desenhar_lista(partida->petiscos, partida->tempoJogado);
}
