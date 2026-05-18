#include "sound.h"
#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#endif
#include <stdio.h>

static const char *sound_paths[SOUND_COUNT] = {
    "resources/sounds/bubbles.wav",
    "resources/sounds/splash.wav",
    "resources/sounds/bite.wav",
    "resources/sounds/hook_cast.wav",
    "resources/sounds/victory.wav",
    "resources/sounds/game_over.wav",
    "resources/sounds/shark_alert.wav"
};

void sound_init(void) {
    /* Rien de spécial pour winmm, mais utile si on passe à GStreamer plus tard */
}

void sound_play(SoundId id) {
    if (id < 0 || id >= SOUND_COUNT) return;
    sound_play_file(sound_paths[id]);
}

void sound_play_file(const char *filename) {
#ifdef _WIN32
    char full_path[512];
    
    /* On retire "resources/" du début si présent */
    const char *rel_path = filename;
    if (g_str_has_prefix(rel_path, "resources/")) {
        rel_path += 10;
    }

    snprintf(full_path, sizeof(full_path), "%s/%s", RESOURCES_PATH, rel_path);

    /* Vérifier si le fichier existe pour éviter un bip système silencieux ou erreur */
    if (GetFileAttributesA(full_path) == INVALID_FILE_ATTRIBUTES) {
        g_warning("Fichier son introuvable: %s", full_path);
        return;
    }
    
    /* PlaySoundA : SND_FILENAME pour un fichier, SND_ASYNC pour ne pas bloquer le jeu */
    PlaySoundA(full_path, NULL, SND_FILENAME | SND_ASYNC);
#else
    g_warning("Lecture de son non implémentée sur cette plateforme : %s", filename);
#endif
}
