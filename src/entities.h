#ifndef ENTITIES_H
#define ENTITIES_H

#include <gtk/gtk.h>

/* ── Types ─────────────────────────────────────────── */
typedef enum {
    FISH_NORMAL, FISH_CLOWN, FISH_BALLOON, FISH_BLUE,
    FISH_YELLOW, FISH_RED, FISH_LANTERN, FISH_MANDARIN,
    FISH_GHOST, FISH_BUBBLE,
    TURTLE_SEA, TURTLE_GIANT, TURTLE_LEATHERBACK,
    SHARK_WHITE, SHARK_WHALE, SHARK_HAMMER,
    SHARK_MAKO, ORCA, SHARK_TIGER, SHARK_GHOST,
    SQUID, OCTOPUS, JELLYFISH, STARFISH,
    CRAB, LOBSTER, DOLPHIN, WHALE,
    SHRIMP, STONEFISH, EEL, MANTA_RAY,
    NARWHAL, CRYSTAL_FISH,
    OBJ_ROCK, OBJ_ALGAE, OBJ_CORAL,
    OBJ_CURRENT, OBJ_WARM, OBJ_COLD,
    OBJ_LIGHT, OBJ_CAVE, OBJ_ANCHOR, OBJ_WRECK,
    ENTITY_COUNT
} EntityType;

typedef enum {
    COLOR_RED, COLOR_BLUE, COLOR_YELLOW,
    COLOR_GREEN, COLOR_ORANGE, COLOR_WHITE,
    COLOR_PURPLE, COLOR_CYAN
} EntityColor;

typedef enum {
    SIZE_TINY, SIZE_SMALL, SIZE_MEDIUM,
    SIZE_LARGE, SIZE_HUGE
} EntitySize;

typedef enum {
    STATE_IDLE, STATE_SWIMMING, STATE_FLEEING,
    STATE_CHASING, STATE_EATING, STATE_DEAD,
    STATE_INFLATED, STATE_INVISIBLE
} EntityState;

/* ── Config statique (par type) ────────────────────── */
typedef struct {
    EntityType   type;
    const char  *name_fr;
    float        base_size;      /* 1.0 = normal */
    float        base_speed;     /* pixels/frame  */
    int          health;
    int          score_value;
    int          danger_level;   /* 0=neutre, 5=très dangereux */
    gboolean     is_predator;
    gboolean     is_collectible;
    gboolean     has_special;
    gboolean     is_object;      /* objet statique (rocher, algue…) */
} EntityConfig;

/* ── Instance vivante ───────────────────────────────── */
typedef struct _Entity Entity;
struct _Entity {
    EntityType   type;
    EntityColor  color;
    EntitySize   size;
    EntityState  state;

    float        x, y;          /* position centre */
    float        vx, vy;        /* vélocité        */
    float        angle;         /* orientation (radians) */
    float        anim_phase;    /* pour les animations cycliques */

    int          health;
    int          score_value;
    float        speed_mult;    /* multiplicateur de vitesse (powerup) */
    float        size_mult;     /* 1.0 normal, >1 gonflé (ballon) */
    float        alpha;         /* transparence 0..1 */

    gboolean     fearful;       /* fuit les requins */
    gboolean     aggressive;    /* attaque les petits */
    gboolean     is_leader;     /* chef de banc */
    Entity      *leader;        /* pointeur vers le chef de banc */

    gboolean     active;        /* FALSE = à supprimer */
};

/* ── API ────────────────────────────────────────────── */
const EntityConfig *entity_get_config(EntityType type);
Entity             *entity_create(EntityType type, float x, float y);
void                entity_free(Entity *e);
void                entity_reset_stats(Entity *e);

/* Retourne TRUE si le type est un prédateur */
gboolean            entity_is_predator(EntityType type);
/* Retourne TRUE si c'est un objet statique */
gboolean            entity_is_object(EntityType type);

#endif /* ENTITIES_H */