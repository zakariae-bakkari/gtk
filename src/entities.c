#include "entities.h"
#include <stdlib.h>

/* Table statique des 44 entités */
static const EntityConfig ENTITY_TABLE[] = {
/* type,              name_fr,            size, speed, hp, pts, danger, pred,  coll,  spec,  obj  */
{FISH_NORMAL,        "Poisson Normal",    1.0f, 2.5f,  1,  10,  0, FALSE, FALSE, FALSE, FALSE},
{FISH_CLOWN,         "Poisson Clown",     0.7f, 2.0f,  1,  15,  0, FALSE, FALSE, TRUE,  FALSE},
{FISH_BALLOON,       "Poisson Ballon",    0.8f, 1.5f,  1,   5,  0, FALSE, FALSE, TRUE,  FALSE},
{FISH_BLUE,          "Poisson Bleu",      1.0f, 4.5f,  1,  20,  0, FALSE, FALSE, TRUE,  FALSE},
{FISH_YELLOW,        "Poisson Jaune",     0.6f, 3.0f,  1,   8,  0, FALSE, FALSE, FALSE, FALSE},
{FISH_RED,           "Poisson Rouge",     0.7f, 2.0f,  1,  12,  0, FALSE, FALSE, FALSE, FALSE},
{FISH_LANTERN,       "Poisson Lanterne",  0.6f, 2.0f,  1,  25,  0, FALSE, FALSE, TRUE,  FALSE},
{FISH_MANDARIN,      "Poisson Mandarin",  0.5f, 1.8f,  1,  30,  0, FALSE, TRUE,  TRUE,  FALSE},
{FISH_GHOST,         "Poisson Fantôme",   1.0f, 2.5f,  1,  40,  0, FALSE, FALSE, TRUE,  FALSE},
{FISH_BUBBLE,        "Poisson Bulle",     0.6f, 1.0f,  1,  10,  0, FALSE, FALSE, FALSE, FALSE},
{TURTLE_SEA,         "Tortue de Mer",     2.0f, 0.8f,  3,  50,  0, FALSE, FALSE, FALSE, FALSE},
{TURTLE_GIANT,       "Tortue Géante",     3.0f, 0.3f,  5,  80,  0, FALSE, FALSE, FALSE, FALSE},
{TURTLE_LEATHERBACK, "Tortue Luth",       2.5f, 0.6f,  3,  60,  0, FALSE, FALSE, FALSE, FALSE},
{SHARK_WHITE,        "Requin Blanc",      2.5f, 4.0f,  5,   0,  4, TRUE,  FALSE, FALSE, FALSE},
{SHARK_WHALE,        "Requin Baleine",    5.0f, 1.5f,  8,   0,  2, TRUE,  FALSE, TRUE,  FALSE},
{SHARK_HAMMER,       "Requin Marteau",    2.5f, 3.0f,  5,   0,  3, TRUE,  FALSE, FALSE, FALSE},
{SHARK_MAKO,         "Requin Mako",       2.0f, 6.0f,  3,   0,  5, TRUE,  FALSE, FALSE, FALSE},
{ORCA,               "Orque",             3.5f, 3.5f,  6,   0,  4, TRUE,  FALSE, TRUE,  FALSE},
{SHARK_TIGER,        "Requin Tigre",      2.5f, 3.5f,  5,   0,  4, TRUE,  FALSE, FALSE, FALSE},
{SHARK_GHOST,        "Requin Fantôme",    2.0f, 3.0f,  4,   0,  3, TRUE,  FALSE, TRUE,  FALSE},
{SQUID,              "Calmar",            1.5f, 3.0f,  2,  20,  1, FALSE, FALSE, TRUE,  FALSE},
{OCTOPUS,            "Pieuvre",           1.5f, 1.5f,  3,  25,  1, FALSE, FALSE, TRUE,  FALSE},
{JELLYFISH,          "Méduse",            0.8f, 0.5f,  1,   5,  2, FALSE, FALSE, TRUE,  FALSE},
{STARFISH,           "Étoile de Mer",     0.5f, 0.0f,  1,   0,  0, FALSE, TRUE,  FALSE, FALSE},
{CRAB,               "Crabe",             0.8f, 0.8f,  2,  15,  1, FALSE, FALSE, FALSE, FALSE},
{LOBSTER,            "Homard",            1.0f, 0.7f,  2,  20,  2, FALSE, FALSE, FALSE, FALSE},
{DOLPHIN,            "Dauphin",           2.5f, 5.0f,  4,   0,  0, FALSE, FALSE, TRUE,  FALSE},
{WHALE,              "Baleine",           6.0f, 0.5f, 10,   0,  0, FALSE, FALSE, TRUE,  FALSE},
{SHRIMP,             "Crevette",          0.3f, 2.5f,  1,   5,  0, FALSE, TRUE,  FALSE, FALSE},
{STONEFISH,          "Poisson Pierre",    0.8f, 0.0f,  2,  10,  3, FALSE, FALSE, TRUE,  FALSE},
{EEL,                "Anguille Élec.",    2.0f, 2.0f,  3,  30,  3, FALSE, FALSE, TRUE,  FALSE},
{MANTA_RAY,          "Raie Manta",        4.0f, 2.5f,  4,  40,  0, FALSE, FALSE, TRUE,  FALSE},
{NARWHAL,            "Narval",            3.0f, 3.0f,  5,  60,  0, FALSE, FALSE, TRUE,  FALSE},
{CRYSTAL_FISH,       "Poisson Cristal",   0.5f, 3.0f,  1, 100,  0, FALSE, TRUE,  TRUE,  FALSE},
{OBJ_ROCK,           "Rocher",            2.0f, 0.0f,  0,   0,  0, FALSE, FALSE, FALSE, TRUE },
{OBJ_ALGAE,          "Algue",             1.5f, 0.0f,  0,   0,  0, FALSE, FALSE, FALSE, TRUE },
{OBJ_CORAL,          "Corail",            1.5f, 0.0f,  0,   0,  0, FALSE, FALSE, FALSE, TRUE },
{OBJ_CURRENT,        "Courant Marin",     3.0f, 0.0f,  0,   0,  0, FALSE, FALSE, FALSE, TRUE },
{OBJ_WARM,           "Zone Chaude",       2.0f, 0.0f,  0,   0,  0, FALSE, FALSE, FALSE, TRUE },
{OBJ_COLD,           "Zone Froide",       2.0f, 0.0f,  0,   0,  0, FALSE, FALSE, FALSE, TRUE },
{OBJ_LIGHT,          "Lumière Abyssale",  1.0f, 0.0f,  0,   0,  0, FALSE, FALSE, FALSE, TRUE },
{OBJ_CAVE,           "Grotte",            4.0f, 0.0f,  0,   0,  0, FALSE, FALSE, FALSE, TRUE },
{OBJ_ANCHOR,         "Ancre",             1.5f, 0.0f,  0,   0,  0, FALSE, FALSE, FALSE, TRUE },
{OBJ_WRECK,          "Épave",             5.0f, 0.0f,  0,   0,  0, FALSE, FALSE, FALSE, TRUE },
};

const EntityConfig *entity_get_config(EntityType type) {
    if (type < 0 || type >= ENTITY_COUNT) return NULL;
    return &ENTITY_TABLE[type];
}

Entity *entity_create(EntityType type, float x, float y) {
    const EntityConfig *cfg = entity_get_config(type);
    if (!cfg) return NULL;

    Entity *e = g_new0(Entity, 1);
    e->type        = type;
    e->color       = COLOR_BLUE;
    e->size        = SIZE_MEDIUM;
    e->state       = STATE_IDLE;
    e->x           = x;
    e->y           = y;
    e->vx          = 0.0f;
    e->vy          = 0.0f;
    e->angle       = 0.0f;
    e->anim_phase  = (float)(rand() % 628) / 100.0f; /* phase aléatoire */
    e->health      = cfg->health;
    e->score_value = cfg->score_value;
    e->speed_mult  = 1.0f;
    e->size_mult   = 1.0f;
    e->alpha       = 1.0f;
    e->fearful     = FALSE;
    e->aggressive  = FALSE;
    e->is_leader   = FALSE;
    e->leader      = NULL;
    e->active      = TRUE;
    return e;
}

void entity_free(Entity *e) {
    if (e) g_free(e);
}

gboolean entity_is_predator(EntityType type) {
    const EntityConfig *c = entity_get_config(type);
    return c ? c->is_predator : FALSE;
}

gboolean entity_is_object(EntityType type) {
    const EntityConfig *c = entity_get_config(type);
    return c ? c->is_object : FALSE;
}