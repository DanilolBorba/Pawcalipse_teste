/* partida.c - Uma partida: cartas, petiscos, colocação de cachorros, pá,
 * combate contra os gatos e tutorial. */

#include "partida.h"
#include "cenario.h"
#include "config.h"
#include "habilidade.h"
#include "ui.h"

#include <stdlib.h>

#define TAMANHO(v)       ((int)(sizeof(v) / sizeof((v)[0])))
#define INTERVALO_CEU    8.0f   // segundos mínimos entre petiscos do céu
#define DURACAO_AVISO    1.6f
#define DURACAO_EFEITO   0.35f
#define PETISCOS_TESTE   100    // ganhos com a tecla ']' (atalho de teste)

static const Rectangle RET_CONTADOR = { 8, 8, 56, 52 };
static const Rectangle RET_PA = { 380, 8, 46, 52 };

static const TipoCachorro deckTutorial[] = { TIPO_POTE_RACAO, TIPO_BEAGLE, TIPO_BULLDOG };
static const TipoCachorro decksTeste[2][MAX_CARTAS] = {   // provisórios, até a escolha de deck (Etapa 6)
    { TIPO_POTE_RACAO, TIPO_BEAGLE, TIPO_BULLDOG, TIPO_DALMATA, TIPO_HUSKY, TIPO_CHIHUAHUA },
    { TIPO_POTE_RACAO, TIPO_PASTOR_ALEMAO, TIPO_SAO_BERNARDO, TIPO_DACHSHUND, TIPO_ROTTWEILER, TIPO_BEAGLE },
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

/* montar_deck: copia os tipos do deck para as cartas da partida, com todas
 * prontas para comprar e nenhuma selecionada. */
static void montar_deck(Partida *partida, const TipoCachorro deck[], int total)
{
    partida->totalCartas = total;
    partida->cartaSelecionada = -1;
    for (int i = 0; i < total; i++) partida->cartas[i] = (Carta){ deck[i], 0.0f };
}

/* partida_criar: aloca a partida com malloc, zera tudo e prepara o campo
 * vazio, as listas de petiscos, gatos e bolinhas vazias, o saldo inicial,
 * as hordas da fase (o tutorial não tem) e o deck (fixo por enquanto: o do
 * tutorial ou o 1º deck de teste). Retorna NULL se faltar memória. */
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
    partida->timerPetiscoCeu = (fase == FASE_TUTORIAL) ? 1.0f : 5.0f;
    partida->resultado = PARTIDA_EM_ANDAMENTO;

    if (fase == FASE_TUTORIAL) {
        montar_deck(partida, deckTutorial, TAMANHO(deckTutorial));
    } else {
        montar_deck(partida, decksTeste[0], MAX_CARTAS);
        horda_iniciar(&partida->hordas, fase);
    }
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

/* agir_cachorro: conta o tempo até a próxima ação do cachorro; quando ele
 * chega a zero, usa a habilidade da raça e, se ela agiu, recomeça a contar
 * a cadência. Se não agiu (sem alvo), tenta de novo no próximo frame. */
static void agir_cachorro(Partida *partida, Cachorro *cachorro, float dt)
{
    cachorro->timer -= dt;
    if (cachorro->timer > 0.0f) return;
    if (habilidade_usar(partida, cachorro)) cachorro->timer = cachorro_info(cachorro->tipo)->cadencia;
}

/* agir_cachorros: percorre a matriz do mapa fazendo cada cachorro agir e
 * remove os que ficaram sem vida (as minas e o Rottweiler, que explodem). */
static void agir_cachorros(Partida *partida, float dt)
{
    for (int l = 0; l < LINHAS_MAPA; l++) {
        for (int c = 0; c < COLUNAS_MAPA; c++) {
            if (partida->mapa.grade[l][c] == NULL) continue;
            agir_cachorro(partida, partida->mapa.grade[l][c], dt);
            if (partida->mapa.grade[l][c]->vida <= 0) mapa_remover(&partida->mapa, l, c);
        }
    }
}

/* partida_adicionar_efeito: ocupa uma vaga livre do vetor de efeitos com um
 * círculo (explosão ou cura). Se todas estiverem ocupadas, não mostra. */
void partida_adicionar_efeito(Partida *partida, Vector2 centro, float raio, Color cor)
{
    for (int i = 0; i < MAX_EFEITOS; i++) {
        if (partida->efeitos[i].tempo > 0.0f) continue;
        partida->efeitos[i] = (Efeito){ centro, raio, DURACAO_EFEITO, cor };
        return;
    }
}

/* desenhar_efeitos: cada efeito ativo é um círculo que cresce até o raio
 * dele enquanto vai ficando transparente. */
static void desenhar_efeitos(const Partida *partida)
{
    for (int i = 0; i < MAX_EFEITOS; i++) {
        const Efeito *e = &partida->efeitos[i];
        if (e->tempo <= 0.0f) continue;

        float resta = e->tempo / DURACAO_EFEITO;
        float raio = e->raio * (1.0f - resta * 0.5f);
        DrawCircleV(e->centro, raio, Fade(e->cor, 0.45f * resta));
        DrawCircleLinesV(e->centro, raio, Fade(e->cor, resta));
    }
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

/* tratar_atalhos_teste: atalhos PROVISÓRIOS para testar as raças: ']' dá
 * PETISCOS_TESTE petiscos (lido pelo caractere digitado, então funciona em
 * qualquer teclado) e, fora do tutorial, TAB troca o deck de teste. */
static void tratar_atalhos_teste(Partida *partida)
{
    for (int tecla = GetCharPressed(); tecla != 0; tecla = GetCharPressed()) {
        if (tecla == ']') partida->saldo += PETISCOS_TESTE;
    }
    if (IsKeyPressed(KEY_TAB) && !partida_eh_tutorial(partida)) {
        partida->deckTeste = 1 - partida->deckTeste;
        montar_deck(partida, decksTeste[partida->deckTeste], MAX_CARTAS);
    }
}

/* tratar_entrada: lê os atalhos de teste, o teclado (1 a 6 escolhem cartas,
 * S pega a pá, botão direito cancela) e o clique esquerdo, nesta ordem de
 * prioridade: coletar petisco, escolher carta, pegar a pá e, por fim, agir
 * no campo (colocar o cachorro selecionado ou remover com a pá). */
static void tratar_entrada(Partida *partida, Vector2 mouse)
{
    tratar_atalhos_teste(partida);
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

/* partida_atualizar: avança o tempo da partida, o aviso, os efeitos e a
 * recarga das cartas; faz os cachorros agirem, manda os gatos das hordas
 * (fora do tutorial), move as bolinhas e os gatos, gera e move os petiscos
 * e trata a entrada do jogador. Se um gato invadir a casa, é derrota; se as
 * 4 hordas forem vencidas, vitória. Com o resultado decidido, a partida
 * congela. */
void partida_atualizar(Partida *partida, Vector2 mouse, float dt)
{
    if (partida->resultado != PARTIDA_EM_ANDAMENTO) return;

    partida->tempoJogado += dt;
    if (partida->timerAviso > 0.0f) partida->timerAviso -= dt;
    for (int i = 0; i < MAX_EFEITOS; i++) {
        if (partida->efeitos[i].tempo > 0.0f) partida->efeitos[i].tempo -= dt;
    }
    for (int i = 0; i < partida->totalCartas; i++) {
        if (partida->cartas[i].recarga > 0.0f) partida->cartas[i].recarga -= dt;
    }

    agir_cachorros(partida, dt);
    if (!partida_eh_tutorial(partida)) horda_atualizar(&partida->hordas, &partida->gatos, dt);
    projetil_atualizar_lista(&partida->projeteis, partida->gatos, dt);
    if (gato_atualizar_lista(&partida->gatos, &partida->mapa, dt))                partida->resultado = PARTIDA_DERROTA;
    else if (!partida_eh_tutorial(partida) && horda_terminou(&partida->hordas)) partida->resultado = PARTIDA_VITORIA;

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
    cachorro_desenhar(carta->tipo, (Vector2){ r.x + r.width / 2, r.y + 20 }, 13, 0, WHITE);
    ui_texto_centro(TextFormat("%d", info->custo), r.x + r.width / 2, r.y + 40, FONTE_PEQUENA, podeComprar ? COR_TEXTO : COR_ALERTA);

    if (carta->recarga > 0.0f) DrawRectangle(r.x, r.y, r.width, r.height * carta->recarga / info->recarga, Fade(BLACK, 0.55f));
    if (!podeComprar) DrawRectangleRec(r, Fade(BLACK, 0.25f));
    DrawRectangleLinesEx(r, 2, (partida->cartaSelecionada == i) ? COR_INFECTADO : COR_BORDA);
}

/* desenhar_progresso: "Horda N/4" e uma barra que enche conforme os gatos
 * da fase vão saindo (não aparece no tutorial). */
static void desenhar_progresso(const Partida *partida)
{
    if (partida_eh_tutorial(partida)) return;

    Rectangle barra = { 440, 43, 186, 5 };
    ui_texto(TextFormat("Horda %d/%d", horda_numero(&partida->hordas), HORDAS_POR_FASE), 566, 30, FONTE_PEQUENA, COR_TEXTO);
    DrawRectangleRec(barra, COR_SOMBRA);
    DrawRectangle(barra.x, barra.y, barra.width * horda_progresso(&partida->hordas), barra.height, COR_SANGUE);
}

/* desenhar_hud: barra superior com o contador de petiscos, as cartas, o
 * botão da pá e, à direita, a fase, o tempo, o progresso das hordas e o
 * nome da carta sob o mouse. */
static void desenhar_hud(const Partida *partida, Vector2 mouse)
{
    ui_desenhar_painel((Rectangle){ 4, 4, 632, 60 });
    DrawRectangleRec(RET_CONTADOR, (Color){ 52, 36, 30, 255 });
    petisco_desenhar_biscoito((Vector2){ 36, 24 }, 2);
    ui_texto_centro(TextFormat("%d", partida->saldo), 36, 38, FONTE_MEDIA, COR_TEXTO);

    for (int i = 0; i < partida->totalCartas; i++) desenhar_carta(partida, i);

    DrawRectangleRec(RET_PA, partida->paSelecionada ? (Color){ 120, 150, 90, 255 } : (Color){ 110, 100, 96, 255 });
    DrawRectangleLinesEx(RET_PA, 2, partida->paSelecionada ? COR_INFECTADO : COR_BORDA);
    ui_texto_centro("Pá", RET_PA.x + 23, 18, FONTE_MEDIA, COR_TEXTO);
    ui_texto_centro("(S)", RET_PA.x + 23, 42, FONTE_PEQUENA, COR_TEXTO);

    int s = (int)partida->tempoJogado;
    ui_texto(partida_eh_tutorial(partida) ? "TUTORIAL" : TextFormat("FASE %d", partida->fase), 440, 8, FONTE_MEDIA, COR_INFECTADO);
    ui_texto(TextFormat("Tempo: %02d:%02d", s / 60, s % 60), 440, 30, FONTE_PEQUENA, COR_TEXTO);
    desenhar_progresso(partida);

    const char *dica = "ESC: pausa";
    for (int i = 0; i < partida->totalCartas; i++) {
        const InfoCachorro *info = cachorro_info(partida->cartas[i].tipo);
        if (CheckCollisionPointRec(mouse, retangulo_carta(i))) dica = TextFormat("%s - %s", info->nome, info->papel);
    }
    ui_texto(dica, 440, 51, FONTE_PEQUENA, COR_BLOQUEADO);
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
        cachorro_desenhar(partida->cartas[partida->cartaSelecionada].tipo, mapa_centro_celula(linha, coluna), RAIO_CACHORRO, 0, Fade(WHITE, 0.5f));
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

/* desenhar_anuncio_horda: pouco antes de cada horda, escreve o anúncio dela
 * em letras grandes no meio do campo. */
static void desenhar_anuncio_horda(const Partida *partida)
{
    const char *anuncio = horda_anuncio(&partida->hordas);
    if (anuncio != NULL) ui_texto_centro(anuncio, LARGURA_VIRTUAL / 2, 200, FONTE_GRANDE, COR_ALERTA);
}

/* partida_desenhar: desenha, de trás para a frente, o céu e os prédios atrás
 * da barra, o campo com os cachorros, os gatos, as bolinhas, as explosões e
 * curas, a prévia sob o mouse, a barra superior, a faixa de mensagens, o
 * anúncio da horda e os petiscos (por cima de tudo). */
void partida_desenhar(const Partida *partida, Vector2 mouse)
{
    cenario_desenhar_ceu(MAPA_Y, partida->tempoJogado);
    cenario_desenhar_skyline_distante(MAPA_Y);
    mapa_desenhar(&partida->mapa);
    gato_desenhar_lista(partida->gatos);
    projetil_desenhar_lista(partida->projeteis);
    desenhar_efeitos(partida);
    desenhar_previa(partida, mouse);
    desenhar_hud(partida, mouse);
    desenhar_mensagem(partida);
    desenhar_anuncio_horda(partida);
    petisco_desenhar_lista(partida->petiscos, partida->tempoJogado);
}
