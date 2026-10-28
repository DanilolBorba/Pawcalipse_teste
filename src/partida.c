/* partida.c - Uma partida: cartas, petiscos, colocação de cachorros, pá,
 * luva, velocidade 2x, combate contra os gatos e tutorial. */

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

#define INFO_X           480    // começo da área de fase, tempo e hordas na barra superior
#define INFO_FIM         586

static const Rectangle RET_CONTADOR = { 8, 8, 56, 52 };
static const Rectangle RET_PA = { 376, 8, 46, 52 };
static const Rectangle RET_LUVA = { 426, 8, 46, 52 };
static const Rectangle RET_ACELERAR = { 592, 8, 40, 52 };

#define LINHA_GATO_TUTORIAL  1      // o gato do tutorial vem pela 2ª linha...
#define X_GATO_TUTORIAL      600.0f // ...já perto do campo, para não demorar

static const TipoCachorro deckTutorial[] = { TIPO_BEAGLE, TIPO_BULLDOG };

enum { TUTORIAL_COLETAR, TUTORIAL_POTE, TUTORIAL_CACHORRO, TUTORIAL_PA, TUTORIAL_GATO };

static const char *textosTutorial[] = {
    "Clique no osso que está caindo para coletar petiscos.",
    "Compre um Pote de Ração: clique na carta e depois no campo.",
    "O pote gera petiscos sozinho! Agora coloque um Beagle.",
    "Use a pá (ou tecla S) para remover um cachorro do campo.",
    "Um gato vem pela 2ª linha! Ponha um Beagle nela. (V acelera)",
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
 * o passo atual, passa para a próxima instrução. Ao chegar ao último passo,
 * solta o gato do tutorial na 2ª linha, já perto do campo. */
static void avancar_tutorial(Partida *partida, int passoCumprido)
{
    if (partida->fase != FASE_TUTORIAL || partida->passoTutorial != passoCumprido) return;

    partida->passoTutorial++;
    if (partida->passoTutorial == TUTORIAL_GATO && gato_adicionar(&partida->gatos, GATO_PADRAO, LINHA_GATO_TUTORIAL)) {
        partida->gatos->x = X_GATO_TUTORIAL;
    }
}

/* montar_deck: monta as cartas da partida: o Pote de Ração e, depois dele,
 * os cachorros do deck, todos prontos para comprar e nenhum selecionado. */
static void montar_deck(Partida *partida, const TipoCachorro deck[], int total)
{
    partida->totalCartas = total + 1;
    partida->cartaSelecionada = -1;
    partida->cartas[0] = (Carta){ TIPO_POTE_RACAO, 0.0f };
    for (int i = 0; i < total; i++) partida->cartas[i + 1] = (Carta){ deck[i], 0.0f };
}

/* partida_criar: aloca a partida com malloc, zera tudo e prepara o campo
 * vazio, as listas de petiscos, gatos e bolinhas vazias, o saldo inicial,
 * a luva com todos os usos e as cartas: no tutorial, um deck fixo; nas
 * fases, o deck escolhido pelo jogador e as hordas. Retorna NULL se faltar
 * memória. */
Partida *partida_criar(int fase, const TipoCachorro deck[TAMANHO_DECK])
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
    partida->usosLuva = USOS_LUVA;
    partida->luvaLinha = -1;

    if (fase == FASE_TUTORIAL) {
        montar_deck(partida, deckTutorial, TAMANHO(deckTutorial));
    } else {
        montar_deck(partida, deck, TAMANHO_DECK);
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

/* cancelar_selecao: desmarca a carta, a pá e a luva (soltando o cachorro
 * que ela tiver pegado, sem gastar um uso). */
static void cancelar_selecao(Partida *partida)
{
    partida->cartaSelecionada = -1;
    partida->paSelecionada = false;
    partida->luvaSelecionada = false;
    partida->luvaLinha = -1;
}

/* selecionar_carta: marca a carta i para ser colocada no campo (desmarcando
 * a pá e a luva). Clicar de novo na mesma carta desmarca. Avisa se ela
 * estiver recarregando ou se não houver petiscos suficientes. */
static void selecionar_carta(Partida *partida, int i)
{
    const Carta *carta = &partida->cartas[i];
    bool jaSelecionada = (partida->cartaSelecionada == i);

    cancelar_selecao(partida);
    if (jaSelecionada) return;
    if (carta->recarga > 0.0f)                                   avisar(partida, "Carta recarregando!");
    else if (partida->saldo < cachorro_info(carta->tipo)->custo) avisar(partida, "Petiscos insuficientes!");
    else                                                         partida->cartaSelecionada = i;
}

/* alternar_pa: pega a pá (desmarcando o resto) ou a larga, se já estava com ela. */
static void alternar_pa(Partida *partida)
{
    bool pegar = !partida->paSelecionada;
    cancelar_selecao(partida);
    partida->paSelecionada = pegar;
}

/* alternar_luva: pega a luva (desmarcando o resto) ou a larga, se já estava
 * com ela. Se os usos acabaram, só avisa. */
static void alternar_luva(Partida *partida)
{
    bool pegar = !partida->luvaSelecionada;
    cancelar_selecao(partida);
    if (pegar && partida->usosLuva <= 0) avisar(partida, "Acabaram os usos da luva!");
    else                                 partida->luvaSelecionada = pegar;
}

/* cachorro_na_luva: retorna o cachorro pego pela luva, ou NULL se ela
 * estiver vazia (ou se ele morreu enquanto estava pego). */
static Cachorro *cachorro_na_luva(const Partida *partida)
{
    if (!partida->luvaSelecionada || partida->luvaLinha < 0) return NULL;
    return partida->mapa.grade[partida->luvaLinha][partida->luvaColuna];
}

/* usar_luva: com a luva vazia, clicar num cachorro o pega (ele só fica
 * marcado). Com um cachorro pego, clicar numa célula livre o muda para lá e
 * gasta um uso; clicar nele mesmo (ou ele ter morrido) o solta sem gastar. */
static void usar_luva(Partida *partida, int linha, int coluna)
{
    if (partida->luvaLinha < 0) {
        if (partida->mapa.grade[linha][coluna] == NULL) return;
        partida->luvaLinha = linha;
        partida->luvaColuna = coluna;
        return;
    }

    Cachorro *cachorro = cachorro_na_luva(partida);
    if (cachorro == NULL || (linha == cachorro->linha && coluna == cachorro->coluna)) {
        partida->luvaLinha = -1;
        return;
    }
    if (!mapa_mover(&partida->mapa, cachorro, linha, coluna)) {
        avisar(partida, "Esse espaço já está ocupado!");
        return;
    }
    partida->usosLuva--;
    cancelar_selecao(partida);
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

/* tratar_atalhos_teste: ATALHOS DE TESTE (saem na versão final, Etapa 10),
 * lidos pelo caractere digitado para funcionarem em qualquer teclado: ']' dá
 * PETISCOS_TESTE petiscos e '\' vence a fase na hora. */
static void tratar_atalhos_teste(Partida *partida)
{
    for (int tecla = GetCharPressed(); tecla != 0; tecla = GetCharPressed()) {
        if (tecla == ']')  partida->saldo += PETISCOS_TESTE;
        if (tecla == '\\') partida->resultado = PARTIDA_VITORIA;
    }
}

/* tratar_teclas: atalhos do teclado: 1 a 6 escolhem cartas, S pega a pá,
 * L pega a luva, V liga/desliga o 2x e o botão direito cancela a seleção. */
static void tratar_teclas(Partida *partida)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) cancelar_selecao(partida);
    for (int i = 0; i < partida->totalCartas; i++) {
        if (IsKeyPressed(KEY_ONE + i)) selecionar_carta(partida, i);
    }
    if (IsKeyPressed(KEY_S)) alternar_pa(partida);
    if (IsKeyPressed(KEY_L)) alternar_luva(partida);
    if (IsKeyPressed(KEY_V)) partida->acelerada = !partida->acelerada;
}

/* clicar_na_barra: trata um clique nos botões da barra superior (cartas,
 * pá, luva e 2x). Retorna true se o clique foi num deles. */
static bool clicar_na_barra(Partida *partida, Vector2 mouse)
{
    for (int i = 0; i < partida->totalCartas; i++) {
        if (CheckCollisionPointRec(mouse, retangulo_carta(i))) {
            selecionar_carta(partida, i);
            return true;
        }
    }
    if (CheckCollisionPointRec(mouse, RET_PA))            alternar_pa(partida);
    else if (CheckCollisionPointRec(mouse, RET_LUVA))     alternar_luva(partida);
    else if (CheckCollisionPointRec(mouse, RET_ACELERAR)) partida->acelerada = !partida->acelerada;
    else return false;
    return true;
}

/* clicar_no_campo: age na célula clicada conforme o que está selecionado:
 * remove o cachorro com a pá, move com a luva ou coloca o da carta. */
static void clicar_no_campo(Partida *partida, int linha, int coluna)
{
    if (partida->paSelecionada) {
        if (partida->mapa.grade[linha][coluna] != NULL) avancar_tutorial(partida, TUTORIAL_PA);
        mapa_remover(&partida->mapa, linha, coluna);
        partida->paSelecionada = false;
    } else if (partida->luvaSelecionada) {
        usar_luva(partida, linha, coluna);
    } else if (partida->cartaSelecionada >= 0) {
        colocar_cachorro(partida, linha, coluna);
    }
}

/* tratar_entrada: lê os atalhos (de teste e do jogo) e o clique esquerdo,
 * nesta ordem de prioridade: coletar petisco, botões da barra superior e,
 * por fim, a célula do campo. */
static void tratar_entrada(Partida *partida, Vector2 mouse)
{
    tratar_atalhos_teste(partida);
    tratar_teclas(partida);
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;

    int valor = petisco_coletar(&partida->petiscos, mouse);
    if (valor > 0) {
        partida->saldo += valor;
        avancar_tutorial(partida, TUTORIAL_COLETAR);
        return;
    }
    if (clicar_na_barra(partida, mouse)) return;

    int linha, coluna;
    if (mapa_celula_no_ponto(mouse, &linha, &coluna)) clicar_no_campo(partida, linha, coluna);
}

/* fase_vencida: true quando as 4 hordas da fase foram vencidas ou, no
 * tutorial, quando o gato do último passo foi derrotado. */
static bool fase_vencida(const Partida *partida)
{
    if (partida_eh_tutorial(partida)) return partida->passoTutorial == TUTORIAL_GATO && partida->gatos == NULL;
    return horda_terminou(&partida->hordas);
}

/* partida_atualizar: avança o tempo da partida, o aviso, os efeitos e a
 * recarga das cartas; faz os cachorros agirem, manda os gatos das hordas
 * (fora do tutorial), move as bolinhas e os gatos, gera e move os petiscos
 * e trata a entrada do jogador. Se um gato invadir a casa, é derrota; se a
 * fase for vencida, vitória. Com o resultado decidido, a partida congela.
 * Com o 2x ligado, o jogo avança o dobro ('passo'), mas o relógio da
 * partida e os avisos seguem o tempo real ('dt'). */
void partida_atualizar(Partida *partida, Vector2 mouse, float dt)
{
    if (partida->resultado != PARTIDA_EM_ANDAMENTO) return;

    float passo = partida->acelerada ? dt * 2.0f : dt;
    partida->tempoJogado += dt;
    if (partida->timerAviso > 0.0f) partida->timerAviso -= dt;
    for (int i = 0; i < MAX_EFEITOS; i++) {
        if (partida->efeitos[i].tempo > 0.0f) partida->efeitos[i].tempo -= passo;
    }
    for (int i = 0; i < partida->totalCartas; i++) {
        if (partida->cartas[i].recarga > 0.0f) partida->cartas[i].recarga -= passo;
    }

    agir_cachorros(partida, passo);
    if (!partida_eh_tutorial(partida)) horda_atualizar(&partida->hordas, &partida->gatos, passo);
    projetil_atualizar_lista(&partida->projeteis, partida->gatos, passo);
    if (gato_atualizar_lista(&partida->gatos, &partida->mapa, passo)) partida->resultado = PARTIDA_DERROTA;
    else if (fase_vencida(partida))                                   partida->resultado = PARTIDA_VITORIA;

    soltar_petisco_do_ceu(partida, passo);
    petisco_atualizar_lista(&partida->petiscos, passo);
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

/* desenhar_botao: fundo e borda de um botão da barra superior (pá, luva,
 * 2x): verde e com borda clara enquanto estiver ativo. */
static void desenhar_botao(Rectangle r, bool ativo)
{
    DrawRectangleRec(r, ativo ? (Color){ 120, 150, 90, 255 } : (Color){ 110, 100, 96, 255 });
    DrawRectangleLinesEx(r, 2, ativo ? COR_INFECTADO : COR_BORDA);
}

/* desenhar_ferramentas: os botões da pá e da luva (provisórios, só com o
 * nome e a tecla). A luva mostra um quadradinho por uso que ainda resta e
 * fica escurecida quando os usos acabam. */
static void desenhar_ferramentas(const Partida *partida)
{
    desenhar_botao(RET_PA, partida->paSelecionada);
    ui_texto_centro("Pá", RET_PA.x + 23, 18, FONTE_MEDIA, COR_TEXTO);
    ui_texto_centro("(S)", RET_PA.x + 23, 42, FONTE_PEQUENA, COR_TEXTO);

    desenhar_botao(RET_LUVA, partida->luvaSelecionada);
    ui_texto_centro("Luva", RET_LUVA.x + 23, 16, FONTE_PEQUENA, COR_TEXTO);
    for (int i = 0; i < USOS_LUVA; i++) {
        Color cor = (i < partida->usosLuva) ? COR_PETISCO : COR_SOMBRA;
        DrawRectangle(RET_LUVA.x + 23 - USOS_LUVA * 5 + i * 10 + 1, 30, 8, 8, cor);
    }
    ui_texto_centro("(L)", RET_LUVA.x + 23, 42, FONTE_PEQUENA, COR_TEXTO);
    if (partida->usosLuva <= 0) DrawRectangleRec(RET_LUVA, Fade(BLACK, 0.45f));
}

/* desenhar_botao_acelerar: o botão do 2x, com o ícone de avançar (dois
 * triângulos apontando para a direita); fica verde com o jogo acelerado. */
static void desenhar_botao_acelerar(const Partida *partida)
{
    Rectangle r = RET_ACELERAR;
    float centroY = r.y + 20;

    desenhar_botao(r, partida->acelerada);
    for (int i = 0; i < 2; i++) {
        float x = r.x + 10 + i * 10;
        DrawTriangle((Vector2){ x, centroY - 7 }, (Vector2){ x, centroY + 7 }, (Vector2){ x + 10, centroY }, COR_TEXTO);
    }
    ui_texto_centro("2x (V)", r.x + r.width / 2, r.y + 36, FONTE_PEQUENA, COR_TEXTO);
}

/* desenhar_info: área entre a luva e o 2x: a fase, o tempo e, fora do
 * tutorial, "Horda N/4" com a barra que enche conforme os gatos da fase vão
 * saindo; por último, o lembrete do ESC. */
static void desenhar_info(const Partida *partida)
{
    int s = (int)partida->tempoJogado;
    ui_texto(partida_eh_tutorial(partida) ? "TUTORIAL" : TextFormat("FASE %d", partida->fase), INFO_X, 8, FONTE_MEDIA, COR_INFECTADO);
    ui_texto(TextFormat("%02d:%02d", s / 60, s % 60), INFO_X, 30, FONTE_PEQUENA, COR_TEXTO);
    ui_texto("ESC: pausa", INFO_X, 51, FONTE_PEQUENA, COR_BLOQUEADO);
    if (partida_eh_tutorial(partida)) return;

    const char *horda = TextFormat("Horda %d/%d", horda_numero(&partida->hordas), HORDAS_POR_FASE);
    Rectangle barra = { INFO_X, 43, INFO_FIM - INFO_X, 5 };
    ui_texto(horda, INFO_FIM - MeasureText(horda, FONTE_PEQUENA), 30, FONTE_PEQUENA, COR_TEXTO);
    DrawRectangleRec(barra, COR_SOMBRA);
    DrawRectangle(barra.x, barra.y, barra.width * horda_progresso(&partida->hordas), barra.height, COR_SANGUE);
}

/* desenhar_hud: barra superior com o contador de petiscos, as cartas, a pá,
 * a luva, as informações da partida e o botão do 2x. */
static void desenhar_hud(const Partida *partida)
{
    ui_desenhar_painel((Rectangle){ 4, 4, 632, 60 });
    DrawRectangleRec(RET_CONTADOR, (Color){ 52, 36, 30, 255 });
    petisco_desenhar_biscoito((Vector2){ 36, 24 }, 2);
    ui_texto_centro(TextFormat("%d", partida->saldo), 36, 38, FONTE_MEDIA, COR_TEXTO);

    for (int i = 0; i < partida->totalCartas; i++) desenhar_carta(partida, i);
    desenhar_ferramentas(partida);
    desenhar_info(partida);
    desenhar_botao_acelerar(partida);
}

/* desenhar_dica_carta: com o mouse sobre uma carta, mostra logo abaixo dela
 * uma caixinha com o nome e o papel do cachorro. */
static void desenhar_dica_carta(const Partida *partida, Vector2 mouse)
{
    for (int i = 0; i < partida->totalCartas; i++) {
        if (!CheckCollisionPointRec(mouse, retangulo_carta(i))) continue;

        const InfoCachorro *info = cachorro_info(partida->cartas[i].tipo);
        const char *texto = TextFormat("%s - %s", info->nome, info->papel);
        Rectangle caixa = { retangulo_carta(i).x, 66, MeasureText(texto, FONTE_PEQUENA) + 8, 14 };
        DrawRectangleRec(caixa, Fade(BLACK, 0.85f));
        DrawRectangleLinesEx(caixa, 1, COR_PETISCO);
        ui_texto(texto, caixa.x + 4, caixa.y + 2, FONTE_PEQUENA, COR_TEXTO);
    }
}

/* desenhar_previa: marca de laranja a célula do cachorro pego pela luva e
 * destaca a célula sob o mouse. Com uma carta (ou um cachorro na luva),
 * mostra o cachorro transparente onde ele vai ficar; pinta de vermelho uma
 * célula ocupada onde não dá para colocar, ou de onde a pá vai remover. */
static void desenhar_previa(const Partida *partida, Vector2 mouse)
{
    const Cachorro *naLuva = cachorro_na_luva(partida);
    if (naLuva != NULL) DrawRectangleLinesEx(mapa_retangulo_celula(naLuva->linha, naLuva->coluna), 3, COR_PETISCO);

    int linha, coluna;
    if (!mapa_celula_no_ponto(mouse, &linha, &coluna)) return;

    Rectangle r = mapa_retangulo_celula(linha, coluna);
    bool ocupada = partida->mapa.grade[linha][coluna] != NULL;
    bool selecionando = partida->paSelecionada || partida->luvaSelecionada || partida->cartaSelecionada >= 0;
    int tipo = (partida->cartaSelecionada >= 0) ? (int)partida->cartas[partida->cartaSelecionada].tipo : (naLuva ? (int)naLuva->tipo : -1);

    if (tipo >= 0 && !ocupada) cachorro_desenhar((TipoCachorro)tipo, mapa_centro_celula(linha, coluna), RAIO_CACHORRO, 0, Fade(WHITE, 0.5f));
    if (ocupada && (partida->paSelecionada || tipo >= 0)) DrawRectangleRec(r, Fade(COR_ALERTA, 0.35f));
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
    if (partida_eh_tutorial(partida) && partida->timerAviso <= 0.0f && passo >= TUTORIAL_POTE) {
        Rectangle alvos[] = { { 0 }, retangulo_carta(0), retangulo_carta(1), RET_PA, retangulo_carta(1) };
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
 * curas, a prévia sob o mouse, a barra superior, a faixa de mensagens, a
 * dica da carta sob o mouse, o anúncio da horda e os petiscos (por cima de
 * tudo). */
void partida_desenhar(const Partida *partida, Vector2 mouse)
{
    cenario_desenhar_ceu(MAPA_Y, partida->tempoJogado);
    cenario_desenhar_skyline_distante(MAPA_Y);
    mapa_desenhar(&partida->mapa);
    gato_desenhar_lista(partida->gatos);
    projetil_desenhar_lista(partida->projeteis);
    desenhar_efeitos(partida);
    desenhar_previa(partida, mouse);
    desenhar_hud(partida);
    desenhar_mensagem(partida);
    desenhar_dica_carta(partida, mouse);
    desenhar_anuncio_horda(partida);
    petisco_desenhar_lista(partida->petiscos, partida->tempoJogado);
}
