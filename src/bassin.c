#include "bassin.h"
#include "draw.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ── Prototypes internes ─────────────────────────── */
static gboolean on_draw     (GtkDrawingArea *da, cairo_t *cr, int w, int h, gpointer data);
static gboolean on_tick     (gpointer data);
static void     update_ai   (Bassin *b, double dt);
static void     draw_all    (Bassin *b, cairo_t *cr, int w, int h);

/* ── Création ────────────────────────────────────── */
Bassin *bassin_create(BassinMode mode) {
    Bassin *b = g_new0(Bassin, 1);
    b->mode = mode;
    b->time = 0.0;
    b->nb_entities = 0;
    b->player = NULL;
    b->width  = 800;
    b->height = 600;

    /* Créer les bulles décoratives */
    b->nb_bubbles = 15;
    for (int i = 0; i < b->nb_bubbles; i++) {
        b->bubbles_x[i] = (rand() % 800);
        b->bubbles_y[i] = (rand() % 600);
        b->bubbles_r[i] = 3.0 + (rand() % 8);
        b->bubbles_phase[i] = (rand() % 628) / 100.0;
    }

    b->drawing_area = gtk_drawing_area_new();
    gtk_widget_set_hexpand(b->drawing_area, TRUE);
    gtk_widget_set_vexpand(b->drawing_area, TRUE);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(b->drawing_area),
                                   (GtkDrawingAreaDrawFunc)on_draw, b, NULL);
    return b;
}

void bassin_destroy(Bassin *b) {
    if (!b) return;
    bassin_stop(b);
    bassin_clear(b);
    g_free(b);
}

GtkWidget *bassin_get_widget(Bassin *b) { return b->drawing_area; }

/* ── Gestion des entités ─────────────────────────── */
void bassin_add_entity(Bassin *b, Entity *e) {
    if (b->nb_entities >= BASSIN_MAX_ENTITIES) return;
    b->entities[b->nb_entities++] = e;
}

void bassin_remove_entity(Bassin *b, Entity *e) {
    for (int i = 0; i < b->nb_entities; i++) {
        if (b->entities[i] == e) {
            b->entities[i] = b->entities[--b->nb_entities];
            b->entities[b->nb_entities] = NULL;
            return;
        }
    }
}

void bassin_clear(Bassin *b) {
    for (int i = 0; i < b->nb_entities; i++) {
        entity_free(b->entities[i]);
        b->entities[i] = NULL;
    }
    b->nb_entities = 0;
    b->player = NULL;
}

void bassin_set_mode(Bassin *b, BassinMode mode) { b->mode = mode; }

/* ── Animation ───────────────────────────────────── */
void bassin_start(Bassin *b) {
    if (b->timer_id) return;
    b->timer_id = g_timeout_add(16, on_tick, b); /* ~60fps */
}

void bassin_stop(Bassin *b) {
    if (b->timer_id) {
        g_source_remove(b->timer_id);
        b->timer_id = 0;
    }
}

static gboolean on_tick(gpointer data) {
    Bassin *b = (Bassin *)data;
    b->time += 0.016;

    /* Mettre à jour les phases des bulles */
    for (int i = 0; i < b->nb_bubbles; i++) {
        b->bubbles_y[i] -= 0.4 + b->bubbles_r[i] * 0.1;
        b->bubbles_phase[i] += 0.03;
        if (b->bubbles_y[i] < -20) {
            b->bubbles_y[i] = b->height + 10;
            b->bubbles_x[i] = rand() % b->width;
        }
    }

    update_ai(b, 0.016);

    gtk_widget_queue_draw(b->drawing_area);
    return G_SOURCE_CONTINUE;
}

/* ── IA basique ──────────────────────────────────── */
static void update_ai(Bassin *b, double dt) {
    /* Mise à jour des phases d'animation */
    for (int i = 0; i < b->nb_entities; i++) {
        Entity *e = b->entities[i];
        if (!e || !e->active) continue;
        if (entity_is_object(e->type)) continue;

        e->anim_phase += dt * 2.0;

        /* Nage simple */
        if (e->state == STATE_SWIMMING || e->state == STATE_IDLE) {
            e->x += e->vx * dt * 60.0;
            e->y += e->vy * dt * 60.0;

            /* Rebond sur les bords */
            if (e->x < 20 || e->x > b->width - 20)  { e->vx = -e->vx; e->angle = atan2(e->vy, e->vx); }
            if (e->y < 20 || e->y > b->height - 80)  { e->vy = -e->vy; e->angle = atan2(e->vy, e->vx); }
        }

        /* Comportement de banc : le follower suit le leader */
        if (!e->is_leader && e->leader && e->leader->active) {
            double dx = e->leader->x - e->x;
            double dy = e->leader->y - e->y;
            double dist = sqrt(dx * dx + dy * dy);
            if (dist > 30.0) {
                double speed = entity_get_config(e->type)->base_speed * e->speed_mult;
                e->vx += (dx / dist) * speed * 0.1;
                e->vy += (dy / dist) * speed * 0.1;
                /* Limiter la vitesse */
                double v = sqrt(e->vx * e->vx + e->vy * e->vy);
                if (v > speed) { e->vx = e->vx / v * speed; e->vy = e->vy / v * speed; }
                e->angle = atan2(e->vy, e->vx);
            }
        }
    }
}

/* ── Rendu ───────────────────────────────────────── */
static void draw_all(Bassin *b, cairo_t *cr, int w, int h) {
    /* Couleurs selon le mode */
    double r1, g1, b1_c, r2, g2, b2_c;
    switch (b->mode) {
        case BASSIN_MODE_PREDATEUR:
            r1=0.02; g1=0.05; b1_c=0.10;
            r2=0.04; g2=0.13; b2_c=0.25;
            break;
        case BASSIN_MODE_CREATEUR:
            r1=0.10; g1=0.42; b1_c=0.54;
            r2=0.16; g2=0.75; b2_c=0.75;
            break;
        case BASSIN_MODE_SURVIE:
            r1=0.05; g1=0.23; b1_c=0.43;
            r2=0.10; g2=0.48; b2_c=0.37;
            break;
        default: /* accueil */
            r1=0.04; g1=0.09; b1_c=0.16;
            r2=0.04; g2=0.31; b2_c=0.30;
            break;
    }

    draw_ocean_background(cr, w, h, b->time, r1, g1, b1_c, r2, g2, b2_c);
    draw_water_rays(cr, w, h, b->time);
    draw_sand_bottom(cr, w, h);

    /* Bulles */
    for (int i = 0; i < b->nb_bubbles; i++) {
        double wave_x = b->bubbles_x[i] + sin(b->bubbles_phase[i]) * 8.0;
        draw_bubble(cr, wave_x, b->bubbles_y[i], b->bubbles_r[i], 0.5);
    }

    /* Entités */
    for (int i = 0; i < b->nb_entities; i++) {
        Entity *e = b->entities[i];
        if (!e || !e->active) continue;

        double er, eg, eb_c;
        entity_color_to_rgb(e->color, &er, &eg, &eb_c);
        float sz = entity_get_config(e->type)->base_size * e->size_mult;

        switch (e->type) {
            case SHARK_WHITE: case SHARK_MAKO: case SHARK_HAMMER:
            case SHARK_TIGER: case SHARK_GHOST: case ORCA:
                draw_shark(cr, e->x, e->y, sz, e->angle, e->anim_phase);
                break;
            case TURTLE_SEA: case TURTLE_GIANT: case TURTLE_LEATHERBACK:
                draw_turtle(cr, e->x, e->y, sz, e->angle);
                break;
            case JELLYFISH:
                draw_jellyfish(cr, e->x, e->y, sz, e->anim_phase);
                break;
            case DOLPHIN:
                draw_dolphin(cr, e->x, e->y, sz, e->angle);
                break;
            case OBJ_ROCK:
                draw_rock(cr, e->x, e->y, sz * 12);
                break;
            case OBJ_ALGAE:
                draw_algae(cr, e->x, e->y, sz * 30, sin(b->time * 1.5) * 0.5);
                break;
            case OBJ_CORAL:
                draw_coral(cr, e->x, e->y, 0, er, eg, eb_c);
                break;
            default:
                if (e->alpha < 0.99)
                    cairo_push_group(cr);
                draw_fish(cr, e->x, e->y, sz, er, eg, eb_c, e->angle, e->anim_phase);
                if (e->alpha < 0.99) {
                    cairo_pop_group_to_source(cr);
                    cairo_paint_with_alpha(cr, e->alpha);
                }
                /* Indicateur chef de banc */
                if (e->is_leader) {
                    draw_glow(cr, e->x, e->y - sz * 18, sz * 8, 1.0, 0.9, 0.2, 0.4);
                }
                break;
        }
    }

    /* Vignette pour mode prédateur */
    if (b->mode == BASSIN_MODE_PREDATEUR)
        draw_vignette(cr, w, h, 0.4);
}

static gboolean on_draw(GtkDrawingArea *da, cairo_t *cr, int w, int h, gpointer data) {
    Bassin *b = (Bassin *)data;
    b->width  = w;
    b->height = h;
    draw_all(b, cr, w, h);
    return FALSE;
}

/* ── Save / Load ─────────────────────────────────── */
gboolean bassin_save(Bassin *b, const char *filepath) {
    FILE *f = fopen(filepath, "w");
    if (!f) return FALSE;

    for (int i = 0; i < b->nb_entities; i++) {
        Entity *e = b->entities[i];
        if (!e || !e->active) continue;
        const EntityConfig *cfg = entity_get_config(e->type);
        fprintf(f, "%s %.1f %.1f %d %d %.2f %d %d\n",
                cfg->name_fr, e->x, e->y,
                (int)e->color, (int)e->size,
                entity_get_config(e->type)->base_speed * e->speed_mult,
                (int)e->fearful, (int)e->aggressive);
    }
    fclose(f);
    return TRUE;
}

gboolean bassin_load(Bassin *b, const char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (!f) return FALSE;

    bassin_clear(b);
    char name[64];
    float x, y, speed;
    int color, size, fearful, aggressive;

    while (fscanf(f, "%63s %f %f %d %d %f %d %d",
                  name, &x, &y, &color, &size, &speed, &fearful, &aggressive) == 8) {
        /* Trouver le type par nom */
        EntityType type = FISH_NORMAL;
        for (int t = 0; t < ENTITY_COUNT; t++) {
            if (strcmp(entity_get_config(t)->name_fr, name) == 0) {
                type = (EntityType)t; break;
            }
        }
        Entity *e = entity_create(type, x, y);
        e->color      = (EntityColor)color;
        e->size       = (EntitySize)size;
        e->speed_mult = speed / entity_get_config(type)->base_speed;
        e->fearful    = fearful;
        e->aggressive = aggressive;
        bassin_add_entity(b, e);
    }
    fclose(f);
    return TRUE;
}

int bassin_count_type(Bassin *b, EntityType type) {
    int count = 0;
    for (int i = 0; i < b->nb_entities; i++)
        if (b->entities[i] && b->entities[i]->type == type) count++;
    return count;
}