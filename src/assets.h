#ifndef ASSETS_H
#define ASSETS_H

#include <gtk/gtk.h>
#include "entities.h"

/* ── Assets Kenney ─────────────────────────────────── */
typedef enum {
    ASSET_FISH_BLUE,
    ASSET_FISH_BROWN,
    ASSET_FISH_GREEN,
    ASSET_FISH_GREY,
    ASSET_FISH_ORANGE,
    ASSET_FISH_PINK,
    ASSET_FISH_RED,
    ASSET_BUBBLE,
    ASSET_SEAWEED,
    ASSET_ROCK,
    ASSET_COUNT
} AssetId;

/* ── Gestionnaire ──────────────────────────────────── */
void assets_init(void);
void assets_cleanup(void);

/* Récupère le pixbuf associé à un AssetId */
GdkPixbuf *assets_get_pixbuf(AssetId id);

/* Mappe un EntityType à un AssetId */
AssetId assets_map_entity_type(EntityType type);

#endif /* ASSETS_H */
