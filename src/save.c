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
 * linhas de comentário (#). Cada linha tem 9 valores lidos com sscanf; uma
 * linha mal formada vira um save vazio. Sem arquivo (primeira vez jogando),
 * todos os saves ficam vazios. */
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
        int lidos = sscanf(linha, "%d %d %d %d %d %d %d %d %f", &usado, &fase, &f[0], &f[1], &f[2], &f[3], &f[4],
                           &s->pontuacao, &s->tempoJogado);
        if (lidos != 9) {
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
    fprintf(arquivo, "# usado faseLiberada f1 f2 f3 f4 f5 pontuacao tempo(segundos)\n");
    for (int i = 0; i < TOTAL_SAVES; i++) {
        const Save *s = &saves[i];
        fprintf(arquivo, "%d %d", s->usado, s->faseLiberada);
        for (int f = 0; f < TOTAL_FASES; f++) fprintf(arquivo, " %d", s->faseConcluida[f]);
        fprintf(arquivo, " %d %.2f\n", s->pontuacao, s->tempoJogado);
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
