/* audio.c - Música de fundo: um loop chiptune em Lá menor, sombrio mas com
 * um toque de "caixinha de música", composto aqui mesmo no código.
 *   melodia: onda quadrada com vibrato    arpejo: onda senoidal (oitava acima)
 *   baixo:   onda triangular              bateria: bumbo, caixa e chimbal (ruído)
 * As amostras são somadas num vetor alocado dinamicamente, convertidas para
 * um arquivo WAV em memória e entregues à raylib como stream de música. */

#include "audio.h"

#include <math.h>
#include <stdlib.h>

#define TAXA_AMOSTRAGEM  22050
#define BPM              100
#define PASSOS_COMPASSO  16      // semicolcheias por compasso
#define TOTAL_COMPASSOS  8
#define VOLUME_MUSICA    0.45f
#define TAMANHO_CABECALHO_WAV 44

typedef enum FormaOnda { ONDA_QUADRADA, ONDA_TRIANGULAR, ONDA_SENO } FormaOnda;

typedef struct Nota {
    int midi;      // número da nota MIDI (0 = pausa)
    int duracao;   // em semicolcheias
} Nota;

typedef struct Envelope {
    float ataque;        // segundos até o volume máximo
    float decaimento;    // velocidade da queda após o ataque
    float sustentacao;   // fração do volume mantida (0 a 1)
    float liberacao;     // segundos de fade-out no fim da nota
} Envelope;

static const Nota melodia[] = {   // 8 compassos: Lám, Fá, Sol, Mi, Lám, Fá, Rém, Mi
    { 76, 4 }, { 69, 2 }, { 72, 2 }, { 71, 4 }, { 69, 4 },
    { 72, 4 }, { 77, 4 }, { 76, 2 }, { 74, 2 }, { 72, 4 },
    { 74, 4 }, { 67, 2 }, { 71, 2 }, { 74, 4 }, { 79, 4 },
    { 68, 8 }, { 71, 4 }, { 76, 4 },
    { 81, 4 }, { 76, 2 }, { 72, 2 }, { 69, 4 }, {  0, 4 },
    { 77, 4 }, { 76, 2 }, { 77, 2 }, { 81, 8 },
    { 74, 4 }, { 77, 2 }, { 81, 2 }, { 79, 4 }, { 77, 4 },
    { 76, 8 }, { 74, 2 }, { 72, 2 }, { 71, 4 },
};

static const int acordes[TOTAL_COMPASSOS][3] = {   // matriz: compasso x notas do acorde (MIDI)
    { 69, 72, 76 }, { 65, 69, 72 }, { 67, 71, 74 }, { 64, 68, 71 },
    { 69, 72, 76 }, { 65, 69, 72 }, { 62, 65, 69 }, { 64, 68, 71 },
};

static const int baixo[TOTAL_COMPASSOS] = { 45, 41, 43, 40, 45, 41, 38, 40 };   // nota grave de cada compasso
static const int padraoBaixo[8] = { 0, 0, 12, 0, 0, 12, 7, 12 };                 // semitons somados em cada colcheia
static const int ordemArpejo[4] = { 0, 1, 2, 1 };                                // ordem das notas do acorde no arpejo

static unsigned int estadoRuido = 22222u;

/* frequencia: converte um número de nota MIDI em hertz (Lá 69 = 440 Hz). */
static float frequencia(int midi)
{
    return 440.0f * powf(2.0f, (midi - 69) / 12.0f);
}

/* ruido_branco: gerador xorshift de ruído entre -1 e 1. Sempre produz a
 * mesma sequência, então a bateria soa igual em toda execução. */
static float ruido_branco(void)
{
    estadoRuido ^= estadoRuido << 13;
    estadoRuido ^= estadoRuido >> 17;
    estadoRuido ^= estadoRuido << 5;
    return (estadoRuido / 4294967295.0f) * 2.0f - 1.0f;
}

/* tocar_nota: soma uma nota ao buffer a partir de 'inicio' (segundos), com a
 * forma de onda, o volume, o envelope e o vibrato pedidos. Índices além do
 * fim "dão a volta" para o começo, assim o final da última nota emenda no
 * início e o loop fica sem cortes. */
static void tocar_nota(float *buffer, int total, float inicio, float duracao, float freq,
                       FormaOnda forma, float volume, Envelope env, float vibrato)
{
    int primeira = (int)(inicio * TAXA_AMOSTRAGEM);
    int quantidade = (int)(duracao * TAXA_AMOSTRAGEM);
    float fase = 0.0f;

    for (int i = 0; i < quantidade; i++) {
        float t = (float)i / TAXA_AMOSTRAGEM;
        float f = freq;
        if (vibrato > 0.0f && t > 0.15f) f *= 1.0f + vibrato * sinf(2.0f * PI * 5.5f * t);

        fase += f / TAXA_AMOSTRAGEM;
        fase -= floorf(fase);

        float amostra;
        switch (forma) {
            case ONDA_QUADRADA:   amostra = (fase < 0.25f) ? 1.0f : -1.0f;  break;
            case ONDA_TRIANGULAR: amostra = 4.0f * fabsf(fase - 0.5f) - 1.0f; break;
            default:              amostra = sinf(2.0f * PI * fase);         break;
        }

        float amplitude = (t < env.ataque) ? t / env.ataque : 1.0f;
        amplitude *= env.sustentacao + (1.0f - env.sustentacao) * expf(-t * env.decaimento);
        float restante = duracao - t;
        if (restante < env.liberacao) amplitude *= restante / env.liberacao;

        buffer[(primeira + i) % total] += amostra * amplitude * volume;
    }
}

/* tocar_bumbo: soma ao buffer um bumbo (seno cujo tom cai rapidamente). */
static void tocar_bumbo(float *buffer, int total, float inicio)
{
    int primeira = (int)(inicio * TAXA_AMOSTRAGEM);
    int quantidade = (int)(0.25f * TAXA_AMOSTRAGEM);
    float fase = 0.0f;
    for (int i = 0; i < quantidade; i++) {
        float t = (float)i / TAXA_AMOSTRAGEM;
        fase += (45.0f + 100.0f * expf(-t * 35.0f)) / TAXA_AMOSTRAGEM;
        buffer[(primeira + i) % total] += sinf(2.0f * PI * fase) * 0.5f * expf(-t * 10.0f);
    }
}

/* tocar_caixa: soma ao buffer uma caixa (ruído com um pouco de tom grave). */
static void tocar_caixa(float *buffer, int total, float inicio)
{
    int primeira = (int)(inicio * TAXA_AMOSTRAGEM);
    int quantidade = (int)(0.2f * TAXA_AMOSTRAGEM);
    for (int i = 0; i < quantidade; i++) {
        float t = (float)i / TAXA_AMOSTRAGEM;
        float som = ruido_branco() * 0.15f * expf(-t * 20.0f)
                  + sinf(2.0f * PI * 190.0f * t) * 0.07f * expf(-t * 25.0f);
        buffer[(primeira + i) % total] += som;
    }
}

/* tocar_chimbal: soma ao buffer um chimbal curto. Usa a diferença entre
 * amostras de ruído, que realça os agudos (som mais "metálico"). */
static void tocar_chimbal(float *buffer, int total, float inicio)
{
    int primeira = (int)(inicio * TAXA_AMOSTRAGEM);
    int quantidade = (int)(0.05f * TAXA_AMOSTRAGEM);
    float anterior = 0.0f;
    for (int i = 0; i < quantidade; i++) {
        float t = (float)i / TAXA_AMOSTRAGEM;
        float r = ruido_branco();
        buffer[(primeira + i) % total] += (r - anterior) * 0.03f * expf(-t * 70.0f);
        anterior = r;
    }
}

/* escrever_inteiro: grava 'valor' em 'bytes' bytes, do menos para o mais
 * significativo (little-endian, como o formato WAV exige). */
static void escrever_inteiro(unsigned char *destino, unsigned int valor, int bytes)
{
    for (int i = 0; i < bytes; i++) destino[i] = (unsigned char)((valor >> (8 * i)) & 0xFF);
}

/* gerar_musica_wav: compõe a música inteira (melodia, arpejo, baixo e
 * bateria, compasso por compasso) num vetor de floats e monta um arquivo
 * WAV completo em memória: cabeçalho de 44 bytes + amostras de 16 bits mono.
 * Devolve o tamanho por 'tamanhoWav' e retorna o buffer (liberar com free),
 * ou NULL se faltar memória. */
static unsigned char *gerar_musica_wav(int *tamanhoWav)
{
    const float segPorPasso = 60.0f / BPM / 4.0f;
    const int totalPassos = PASSOS_COMPASSO * TOTAL_COMPASSOS;
    const int total = (int)(totalPassos * segPorPasso * TAXA_AMOSTRAGEM);

    float *mixagem = calloc((size_t)total, sizeof(float));
    if (mixagem == NULL) return NULL;

    const Envelope envMelodia = { 0.005f, 3.0f, 0.65f, 0.04f };
    const Envelope envArpejo  = { 0.002f, 9.0f, 0.10f, 0.05f };
    const Envelope envBaixo   = { 0.004f, 4.0f, 0.55f, 0.03f };

    int passo = 0;
    for (size_t i = 0; i < sizeof(melodia) / sizeof(melodia[0]); i++) {
        if (melodia[i].midi > 0) {
            tocar_nota(mixagem, total, passo * segPorPasso, melodia[i].duracao * segPorPasso,
                       frequencia(melodia[i].midi), ONDA_QUADRADA, 0.10f, envMelodia, 0.004f);
        }
        passo += melodia[i].duracao;
    }

    for (int c = 0; c < TOTAL_COMPASSOS; c++) {
        for (int p = 0; p < PASSOS_COMPASSO; p++) {
            float t = (c * PASSOS_COMPASSO + p) * segPorPasso;

            int notaArpejo = acordes[c][ordemArpejo[p % 4]] + 12;
            tocar_nota(mixagem, total, t, segPorPasso * 2.0f, frequencia(notaArpejo),
                       ONDA_SENO, 0.05f, envArpejo, 0.0f);

            if (p % 2 == 0) {
                int notaBaixo = baixo[c] + padraoBaixo[p / 2];
                tocar_nota(mixagem, total, t, segPorPasso * 2.0f, frequencia(notaBaixo),
                           ONDA_TRIANGULAR, 0.22f, envBaixo, 0.0f);
            }

            if (p == 0 || p == 10) tocar_bumbo(mixagem, total, t);
            if (p == 8)            tocar_caixa(mixagem, total, t);
            if (p % 4 == 2)        tocar_chimbal(mixagem, total, t);
        }
    }

    int bytesDados = total * 2;
    unsigned char *wav = malloc((size_t)TAMANHO_CABECALHO_WAV + bytesDados);
    if (wav == NULL) {
        free(mixagem);
        return NULL;
    }

    unsigned char *h = wav;
    h[0] = 'R'; h[1] = 'I'; h[2] = 'F'; h[3] = 'F';
    escrever_inteiro(h + 4, 36 + bytesDados, 4);
    h[8] = 'W'; h[9] = 'A'; h[10] = 'V'; h[11] = 'E';
    h[12] = 'f'; h[13] = 'm'; h[14] = 't'; h[15] = ' ';
    escrever_inteiro(h + 16, 16, 4);
    escrever_inteiro(h + 20, 1, 2);
    escrever_inteiro(h + 22, 1, 2);
    escrever_inteiro(h + 24, TAXA_AMOSTRAGEM, 4);
    escrever_inteiro(h + 28, TAXA_AMOSTRAGEM * 2, 4);
    escrever_inteiro(h + 32, 2, 2);
    escrever_inteiro(h + 34, 16, 2);
    h[36] = 'd'; h[37] = 'a'; h[38] = 't'; h[39] = 'a';
    escrever_inteiro(h + 40, bytesDados, 4);

    for (int i = 0; i < total; i++) {
        float s = mixagem[i] * 0.9f;
        if (s > 1.0f) s = 1.0f;
        if (s < -1.0f) s = -1.0f;
        short valor = (short)(s * 32767.0f);
        escrever_inteiro(wav + TAMANHO_CABECALHO_WAV + i * 2, (unsigned short)valor, 2);
    }

    free(mixagem);
    *tamanhoWav = TAMANHO_CABECALHO_WAV + bytesDados;
    return wav;
}

/* audio_iniciar: abre o dispositivo de áudio e carrega a música: primeiro
 * tenta um arquivo da equipe em assets/audio/; se não houver, gera a música
 * em memória. Se tudo der certo, toca em loop e marca 'pronto'. */
void audio_iniciar(Audio *audio)
{
    audio->pronto = false;
    audio->mudo = false;
    audio->musica = (Music){ 0 };
    audio->dadosWav = NULL;

    InitAudioDevice();
    if (!IsAudioDeviceReady()) return;

    static const char *arquivos[] = {
        "assets/audio/musica.ogg", "assets/audio/musica.mp3", "assets/audio/musica.wav",
    };
    for (int i = 0; i < 3 && !IsMusicValid(audio->musica); i++) {
        if (FileExists(arquivos[i])) audio->musica = LoadMusicStream(arquivos[i]);
    }

    if (!IsMusicValid(audio->musica)) {
        int tamanho = 0;
        audio->dadosWav = gerar_musica_wav(&tamanho);
        if (audio->dadosWav != NULL) {
            audio->musica = LoadMusicStreamFromMemory(".wav", audio->dadosWav, tamanho);
        }
    }

    if (!IsMusicValid(audio->musica)) {
        free(audio->dadosWav);
        audio->dadosWav = NULL;
        CloseAudioDevice();
        return;
    }

    audio->musica.looping = true;
    SetMusicVolume(audio->musica, VOLUME_MUSICA);
    PlayMusicStream(audio->musica);
    audio->pronto = true;
}

/* audio_atualizar: deve ser chamada todo frame. Liga/desliga o som com a
 * tecla M e alimenta o buffer da música (UpdateMusicStream). */
void audio_atualizar(Audio *audio)
{
    if (!audio->pronto) return;

    if (IsKeyPressed(KEY_M)) {
        audio->mudo = !audio->mudo;
        SetMusicVolume(audio->musica, audio->mudo ? 0.0f : VOLUME_MUSICA);
    }
    UpdateMusicStream(audio->musica);
}

/* audio_finalizar: para e descarrega a música, libera o WAV gerado e fecha
 * o dispositivo de áudio. */
void audio_finalizar(Audio *audio)
{
    if (!audio->pronto) return;

    StopMusicStream(audio->musica);
    UnloadMusicStream(audio->musica);
    free(audio->dadosWav);
    audio->dadosWav = NULL;
    CloseAudioDevice();
    audio->pronto = false;
}
