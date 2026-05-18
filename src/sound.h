#ifndef SOUND_H
#define SOUND_H

#include <gtk/gtk.h>

/* ── Sons du jeu ───────────────────────────────────── */
typedef enum {
    SOUND_BUBBLES,
    SOUND_SPLASH,
    SOUND_BITE,
    SOUND_HOOK_CAST,
    SOUND_VICTORY,
    SOUND_GAME_OVER,
    SOUND_SHARK_ALERT,
    SOUND_COUNT
} SoundId;

/* ── API ────────────────────────────────────────────── */
void sound_init(void);
void sound_play(SoundId id);
void sound_play_file(const char *filename);

#endif /* SOUND_H */
