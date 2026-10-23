#ifndef AUDIO_H
#define AUDIO_H

/* audio.h - Música de fundo. Usa assets/audio/musica.(ogg|mp3|wav) se o
 * arquivo existir; senão, sintetiza uma música chiptune própria em memória. */

#include "raylib.h"
#include <stdbool.h>

typedef struct Audio {
    bool           pronto;    // dispositivo de áudio e música carregados
    bool           mudo;      // alternado com a tecla M
    Music          musica;
    unsigned char *dadosWav;  // música gerada em memória (NULL se veio de arquivo)
} Audio;

void audio_iniciar(Audio *audio);    // abre o áudio e toca a música em loop
void audio_atualizar(Audio *audio);  // todo frame: alimenta a música e trata a tecla M
void audio_finalizar(Audio *audio);  // para a música e libera tudo

#endif
