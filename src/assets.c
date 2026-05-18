#include "assets.h"
#include <stdio.h>

static GdkPixbuf *pixbufs[ASSET_COUNT] = {0};

static const char *asset_paths[ASSET_COUNT] = {
    "resources/kenney_fish-pack_2.0/PNG/Default/fish_blue.png",
    "resources/kenney_fish-pack_2.0/PNG/Default/fish_brown.png",
    "resources/kenney_fish-pack_2.0/PNG/Default/fish_green.png",
    "resources/kenney_fish-pack_2.0/PNG/Default/fish_grey.png",
    "resources/kenney_fish-pack_2.0/PNG/Default/fish_orange.png",
    "resources/kenney_fish-pack_2.0/PNG/Default/fish_pink.png",
    "resources/kenney_fish-pack_2.0/PNG/Default/fish_red.png",
    "resources/kenney_fish-pack_2.0/PNG/Default/bubble_a.png",
    "resources/kenney_fish-pack_2.0/PNG/Default/seaweed_green_a.png",
    "resources/kenney_fish-pack_2.0/PNG/Default/rock_a.png"
};

void assets_init(void) {
    for (int i = 0; i < ASSET_COUNT; i++) {
        GError *error = NULL;
        char full_path[512];
        
        /* On retire "resources/" du début car RESOURCES_PATH le contient déjà */
        const char *rel_path = asset_paths[i];
        if (g_str_has_prefix(rel_path, "resources/")) {
            rel_path += 10;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", RESOURCES_PATH, rel_path);
        
        pixbufs[i] = gdk_pixbuf_new_from_file(full_path, &error);
        if (error) {
            g_warning("Erreur chargement asset %s: %s", full_path, error->message);
            g_error_free(error);
        }
    }
}

void assets_cleanup(void) {
    for (int i = 0; i < ASSET_COUNT; i++) {
        if (pixbufs[i]) {
            g_object_unref(pixbufs[i]);
            pixbufs[i] = NULL;
        }
    }
}

GdkPixbuf *assets_get_pixbuf(AssetId id) {
    if (id < 0 || id >= ASSET_COUNT) return NULL;
    return pixbufs[id];
}

AssetId assets_map_entity_type(EntityType type) {
    switch (type) {
        case FISH_BLUE:    return ASSET_FISH_BLUE;
        case FISH_RED:     return ASSET_FISH_RED;
        case FISH_YELLOW:  return ASSET_FISH_ORANGE;
        case FISH_NORMAL:  return ASSET_FISH_GREEN;
        case FISH_GHOST:   return ASSET_FISH_GREY;
        case FISH_BUBBLE:  return ASSET_BUBBLE;
        case OBJ_ALGAE:    return ASSET_SEAWEED;
        case OBJ_ROCK:     return ASSET_ROCK;
        /* Par défaut */
        default:           return ASSET_FISH_BLUE;
    }
}
