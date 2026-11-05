/* save.c - Leitura e escrita dos saves em arquivo texto. */

#include "save.h"

#include <stdio.h>

/* save_limpar: zera todos os campos do save (espaço vazio, só a fase 1). */
void save_limpar(Save *save)
{
    *save = (Save){ 0 };
    save->faseLiberada = 1;
}

/* save_novo: começa um jogo novo no save: limpo, mas marcado como usado. */
void save_novo(Save *save)
{
    save_limpar(save);
    save->usado = true;
}

/* save_carregar_todos: lê os saves do arquivo, linha por linha, pulando as
 * linhas de comentário (#). Cada linha tem 14 valores lidos com sscanf (ou 9,
 * no formato antigo, sem os recordes, que ficam zerados); uma linha mal
 * formada vira um save vazio. Sem arquivo (primeira vez jogando), todos os
 * saves ficam vazios. */
void save_carregar_todos(Save saves[TOTAL_SAVES])
{
    for (int i = 0; i < TOTAL_SAVES; i++) save_limpar(&saves[i]);

    FILE *arquivo = fopen(ARQUIVO_SAVES, "r");
    if (arquivo == NULL) return;

    char linha[160];
    int i = 0;
    while (i < TOTAL_SAVES && fgets(linha, sizeof(linha), arquivo) != NULL) {
        if (linha[0] == '#' || linha[0] == '\n') continue;

        Save *s = &saves[i++];
        int usado, fase, f[TOTAL_FASES];
        int lidos = sscanf(linha, "%d %d %d %d %d %d %d %d %f %d %d %d %d %d", &usado, &fase, &f[0], &f[1], &f[2], &f[3], &f[4],
                           &s->pontuacao, &s->tempoJogado, &s->recorde[0], &s->recorde[1], &s->recorde[2], &s->recorde[3], &s->recorde[4]);
        if (lidos != 14 && lidos != 9) {
            save_limpar(s);
            continue;
        }
        s->usado = usado;
        s->faseLiberada = (fase >= 1 && fase <= TOTAL_FASES) ? fase : 1;
        for (int k = 0; k < TOTAL_FASES; k++) s->faseConcluida[k] = f[k];
    }
    fclose(arquivo);
}

/* save_gravar_todos: reescreve o arquivo com um cabeçalho de comentário e
 * uma linha por save. Retorna false se não conseguir abrir o arquivo. */
bool save_gravar_todos(const Save saves[TOTAL_SAVES])
{
    FILE *arquivo = fopen(ARQUIVO_SAVES, "w");
    if (arquivo == NULL) return false;

    fprintf(arquivo, "# PawPocalypse - saves\n");
    fprintf(arquivo, "# usado faseLiberada f1 f2 f3 f4 f5 pontuacao tempo(segundos) recorde1 ... recorde5\n");
    for (int i = 0; i < TOTAL_SAVES; i++) {
        const Save *s = &saves[i];
        fprintf(arquivo, "%d %d", s->usado, s->faseLiberada);
        for (int f = 0; f < TOTAL_FASES; f++) fprintf(arquivo, " %d", s->faseConcluida[f]);
        fprintf(arquivo, " %d %.2f", s->pontuacao, s->tempoJogado);
        for (int f = 0; f < TOTAL_FASES; f++) fprintf(arquivo, " %d", s->recorde[f]);
        fprintf(arquivo, "\n");
    }
    fclose(arquivo);
    return true;
}

/* save_fases_concluidas: conta quantas posições de faseConcluida são true. */
int save_fases_concluidas(const Save *save)
{
    int total = 0;
    for (int f = 0; f < TOTAL_FASES; f++) total += save->faseConcluida[f];
    return total;
}

/* save_registrar_pontos: se os pontos passarem do recorde da fase no save,
 * troca o recorde e soma a diferença na pontuação do save (que é sempre a
 * soma dos recordes, para repetir uma fase não inflar os pontos). Retorna
 * true se foi um novo recorde. */
bool save_registrar_pontos(Save *save, int fase, int pontos)
{
    int *recorde = &save->recorde[fase - 1];
    if (pontos <= *recorde) return false;

    save->pontuacao += pontos - *recorde;
    *recorde = pontos;
    return true;
}
