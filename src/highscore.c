/* highscore.c - Leitura, escrita, inserção e desenho do Top 10. */

#include "highscore.h"
#include "save.h"
#include "ui.h"

#include <stdio.h>
#include <string.h>

/* ranking_carregar: zera o ranking e lê o arquivo linha por linha, pulando
 * as linhas de comentário (#). Cada linha tem os pontos, a fase, o tempo e,
 * no fim, o nome (que pode ter espaços), lidos com sscanf; linhas mal
 * formadas são ignoradas. Sem arquivo, o ranking fica vazio. */
void ranking_carregar(Ranking *ranking)
{
    *ranking = (Ranking){ 0 };
    FILE *arquivo = fopen(ARQUIVO_RECORDES, "r");
    if (arquivo == NULL) return;

    char linha[96];
    while (ranking->total < TOTAL_RECORDES && fgets(linha, sizeof(linha), arquivo) != NULL) {
        if (linha[0] == '#' || linha[0] == '\n') continue;

        Recorde *r = &ranking->lista[ranking->total];
        if (sscanf(linha, "%d %d %f %12[^\n]", &r->pontos, &r->fase, &r->tempo, r->nome) == 4) ranking->total++;
    }
    fclose(arquivo);
}

/* ranking_gravar: reescreve o arquivo com um cabeçalho de comentário e uma
 * linha por recorde, já na ordem do ranking. Retorna false se não conseguir
 * abrir o arquivo. */
bool ranking_gravar(const Ranking *ranking)
{
    FILE *arquivo = fopen(ARQUIVO_RECORDES, "w");
    if (arquivo == NULL) return false;

    fprintf(arquivo, "# PawPocalypse - Top 10\n");
    fprintf(arquivo, "# pontos fase tempo(segundos) nome\n");
    for (int i = 0; i < ranking->total; i++) {
        const Recorde *r = &ranking->lista[i];
        fprintf(arquivo, "%d %d %.2f %s\n", r->pontos, r->fase, r->tempo, r->nome);
    }
    fclose(arquivo);
    return true;
}

/* ranking_posicao: procura o 1º lugar com menos pontos (num empate, quem
 * chegou antes fica na frente). Retorna esse lugar, o fim da lista se ainda
 * houver espaço, ou -1 se os pontos não entram no Top 10 (ou forem zero). */
int ranking_posicao(const Ranking *ranking, int pontos)
{
    if (pontos <= 0) return -1;
    for (int i = 0; i < ranking->total; i++) {
        if (pontos > ranking->lista[i].pontos) return i;
    }
    return (ranking->total < TOTAL_RECORDES) ? ranking->total : -1;
}

/* ranking_inserir: acha o lugar dos pontos, empurra os recordes de lá para
 * baixo uma posição (o 10º sai, se a lista estiver cheia) e grava o novo
 * recorde no lugar. Um nome vazio vira "Jogador". Retorna o lugar (0 a 9),
 * ou -1 se os pontos não entram. */
int ranking_inserir(Ranking *ranking, const char *nome, int fase, int pontos, float tempo)
{
    int posicao = ranking_posicao(ranking, pontos);
    if (posicao < 0) return -1;

    if (ranking->total < TOTAL_RECORDES) ranking->total++;
    for (int i = ranking->total - 1; i > posicao; i--) ranking->lista[i] = ranking->lista[i - 1];

    Recorde *novo = &ranking->lista[posicao];
    snprintf(novo->nome, sizeof(novo->nome), "%s", (nome[0] != '\0') ? nome : "Jogador");
    novo->fase = fase;
    novo->pontos = pontos;
    novo->tempo = tempo;
    return posicao;
}

/* ranking_desenhar: dentro da área, o cabeçalho das colunas e uma linha por
 * recorde (o 1º lugar em laranja); sem recordes, uma mensagem. */
void ranking_desenhar(const Ranking *ranking, Rectangle area)
{
    static const int colunas[] = { 0, 30, 170, 240, 320 };
    static const char *titulos[] = { "#", "Nome", "Fase", "Pontos", "Tempo" };

    for (int c = 0; c < 5; c++) ui_texto(titulos[c], area.x + colunas[c], area.y, FONTE_PEQUENA, COR_INFECTADO);
    if (ranking->total == 0) ui_texto_centro("Nenhum recorde ainda. Vença uma fase!", area.x + area.width / 2, area.y + 60, FONTE_PEQUENA, COR_BLOQUEADO);

    for (int i = 0; i < ranking->total; i++) {
        const Recorde *r = &ranking->lista[i];
        int y = area.y + 18 + i * 16, s = (int)r->tempo;
        Color cor = (i == 0) ? COR_PETISCO : COR_TEXTO;

        ui_texto(TextFormat("%dº", i + 1), area.x + colunas[0], y, FONTE_PEQUENA, cor);
        ui_texto(r->nome, area.x + colunas[1], y, FONTE_PEQUENA, cor);
        ui_texto((r->fase == TOTAL_FASES) ? "Chefão" : TextFormat("%d", r->fase), area.x + colunas[2], y, FONTE_PEQUENA, cor);
        ui_texto(TextFormat("%d", r->pontos), area.x + colunas[3], y, FONTE_PEQUENA, cor);
        ui_texto(TextFormat("%02d:%02d", s / 60, s % 60), area.x + colunas[4], y, FONTE_PEQUENA, cor);
    }
}
