#ifndef BASSIN_H
#define BASSIN_H

#include <gtk/gtk.h>
#include "entities.h"

#define BASSIN_MAX_ENTITIES 256

typedef enum {
    BASSIN_MODE_ACCUEIL,   /* animation décorative */
    BASSIN_MODE_CREATEUR,  /* éditeur */
    BASSIN_MODE_PREDATEUR, /* jeu requin */
    BASSIN_MODE_SURVIE     /* jeu poisson */
} BassinMode;

typedef struct {
    GtkWidget   *drawing_area;  /* GtkDrawingArea */
    BassinMode   mode;

    /* Entités */
    Entity      *entities[BASSIN_MAX_ENTITIES];
    int          nb_entities;

    /* Entité joueur */
    Entity      *player;

    /* Animation */
    guint        timer_id;
    double       time;          /* temps global en secondes */

    /* Bulles décoratives */
    double       bubbles_x[20];
    double       bubbles_y[20];
    double       bubbles_r[20];
    double       bubbles_phase[20];
    int          nb_bubbles;

    /* Taille du canvas */
    int          width;
    int          height;

    /* Callbacks */
    void (*on_entity_clicked)(Entity *e, gpointer data);
    void (*on_entity_eaten)  (Entity *e, int score, gpointer data);
    gpointer callback_data;
} Bassin;

/* ── API ─────────────────────────────────────────── */
Bassin    *bassin_create  (BassinMode mode);
void       bassin_destroy (Bassin *b);

GtkWidget *bassin_get_widget(Bassin *b);

void       bassin_add_entity   (Bassin *b, Entity *e);
void       bassin_remove_entity(Bassin *b, Entity *e);
void       bassin_clear        (Bassin *b);

void       bassin_set_mode(Bassin *b, BassinMode mode);
void       bassin_start   (Bassin *b);
void       bassin_stop    (Bassin *b);

/* Sauvegarde / chargement (Mode Créateur) */
gboolean   bassin_save(Bassin *b, const char *filepath);
gboolean   bassin_load(Bassin *b, const char *filepath);

/* Compter les entités par type */
int        bassin_count_type(Bassin *b, EntityType type);

#endif /* BASSIN_H */