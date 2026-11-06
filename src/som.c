/* som.c - Tabela dos efeitos sonoros, carregamento e volume. */

#include "som.h"

static const char *arquivos[TOTAL_SONS] = {
    [SOM_CLIQUE]     = "assets/sons/clique.wav",
    [SOM_CARTA]      = "assets/sons/carta.wav",
    [SOM_INICIAR]    = "assets/sons/iniciar.wav",
    [SOM_COLOCAR]    = "assets/sons/colocar.wav",
    [SOM_PA]         = "assets/sons/pa.wav",
    [SOM_LUVA]       = "assets/sons/luva.wav",
    [SOM_PETISCO]    = "assets/sons/petisco.wav",
    [SOM_RACAO]      = "assets/sons/racao.wav",
    [SOM_CUSPE]      = "assets/sons/cuspe.wav",
    [SOM_LATIDO]     = "assets/sons/latido.wav",
    [SOM_EXPLOSAO]   = "assets/sons/explosao.wav",
    [SOM_ZUMBIDO]    = "assets/sons/zumbido.wav",
    [SOM_MORDIDA]    = "assets/sons/mordida.wav",
    [SOM_GATO_MORRE] = "assets/sons/gato_morre.wav",
    [SOM_RUGIDO]     = "assets/sons/rugido.wav",
    [SOM_ALERTA]     = "assets/sons/alerta.wav",
    [SOM_VITORIA]    = "assets/sons/vitoria.wav",
    [SOM_DERROTA]    = "assets/sons/derrota.wav",
};

static Sound sons[TOTAL_SONS];      // vazio = não carregado (mudo)
static float volume = 0.7f;

/* som_carregar_todos: com o dispositivo de áudio aberto, carrega cada WAV
 * da tabela que existir; os que faltarem ficam vazios. */
void som_carregar_todos(void)
{
    if (!IsAudioDeviceReady()) return;
    for (int i = 0; i < TOTAL_SONS; i++) {
        if (FileExists(arquivos[i])) sons[i] = LoadSound(arquivos[i]);
    }
}

/* som_descarregar_todos: libera os sons carregados e os deixa vazios. */
void som_descarregar_todos(void)
{
    for (int i = 0; i < TOTAL_SONS; i++) {
        if (IsSoundValid(sons[i])) UnloadSound(sons[i]);
        sons[i] = (Sound){ 0 };
    }
}

/* som_tocar: toca o efeito no volume atual. Cada efeito tem uma "voz" só:
 * se ele já estiver tocando, recomeça (assim 20 cachorros cuspindo juntos
 * não viram uma barulheira). Sem o som carregado, não faz nada. */
void som_tocar(IdSom id)
{
    if (!IsSoundValid(sons[id])) return;
    SetSoundVolume(sons[id], volume);
    PlaySound(sons[id]);
}

/* som_definir_volume: muda o volume dos efeitos (limitado entre 0 e 1). */
void som_definir_volume(float novo)
{
    volume = (novo < 0.0f) ? 0.0f : (novo > 1.0f ? 1.0f : novo);
}

/* som_volume: retorna o volume atual dos efeitos. */
float som_volume(void)
{
    return volume;
}
