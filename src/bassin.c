#include "bassin.h"
#include "draw.h"
#include "sound.h"
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
    
    /* Petit son de splash quand on ajoute un poisson */
    if (!entity_is_object(e->type)) {
        sound_play(SOUND_SPLASH);
    }
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
            
            /* Jouer un son de bulle de temps en temps (10% de chance) */
            if (rand() % 10 == 0) {
                sound_play(SOUND_BUBBLES);
            }
        }
    }

    update_ai(b, 0.016);

    gtk_widget_queue_draw(b->drawing_area);
    return G_SOURCE_CONTINUE;
}

/* ── IA avancée (Prédateurs, Proies, Bancs, Fuite, Chasse, Repas) ──────────────── */
gboolean g_creator_paused = FALSE;

static void update_ai(Bassin *b, double dt) {
    /* Si le créateur est en pause, on n'actualise pas les mouvements */
    if (g_creator_paused && b->mode == BASSIN_MODE_CREATEUR) {
        return;
    }

    /* 1. Mise à jour de la phase d'animation de chaque entité active */
    for (int i = 0; i < b->nb_entities; i++) {
        Entity *e = b->entities[i];
        if (!e || !e->active) continue;
        if (entity_is_object(e->type)) continue;

        e->anim_phase += dt * 5.0; // vitesse d'animation
    }

    /* 2. Mouvement et comportement IA */
    for (int i = 0; i < b->nb_entities; i++) {
        Entity *e = b->entities[i];
        if (!e || !e->active) continue;
        if (entity_is_object(e->type)) continue;

        // Le joueur est contrôlé par l'utilisateur (souris ou clavier dans screen_jeux)
        if (b->player == e) {
            // Mais le joueur prédateur peut quand même MANGER les proies autonomes !
            if (entity_is_predator(e->type)) {
                for (int j = 0; j < b->nb_entities; j++) {
                    Entity *other = b->entities[j];
                    if (!other || !other->active || other == e) continue;
                    if (entity_is_predator(other->type) || entity_is_object(other->type)) continue;

                    double dx = other->x - e->x;
                    double dy = other->y - e->y;
                    double dist = sqrt(dx*dx + dy*dy);

                    if (dist < 30.0) { // Distance d'ingestion (eating space)
                        other->active = FALSE;
                        sound_play(SOUND_BITE);
                        if (b->on_entity_eaten) {
                            b->on_entity_eaten(other, entity_get_config(other->type)->score_value, b->callback_data);
                        }
                    }
                }
            }
            continue;
        }

        const EntityConfig *config = entity_get_config(e->type);
        float base_speed = config->base_speed * e->speed_mult;

        if (entity_is_predator(e->type)) {
            /* === COMPORTEMENT DU PRÉDATEUR (REQUIN...) === */
            Entity *nearest_prey = NULL;
            double min_dist = 99999.0;

            // Chercher la proie active la plus proche (qui n'est pas un prédateur ni un objet statique)
            for (int j = 0; j < b->nb_entities; j++) {
                Entity *other = b->entities[j];
                if (!other || !other->active || other == e) continue;
                if (entity_is_predator(other->type) || entity_is_object(other->type)) continue;

                double dx = other->x - e->x;
                double dy = other->y - e->y;
                double dist = sqrt(dx*dx + dy*dy);
                if (dist < min_dist) {
                    min_dist = dist;
                    nearest_prey = other;
                }
            }

            // Si une proie est détectée dans la zone de détection (180 px)
            if (nearest_prey && min_dist < 180.0) {
                e->state = STATE_CHASING;
                double dx = nearest_prey->x - e->x;
                double dy = nearest_prey->y - e->y;
                double speed = base_speed * 1.4f; // Plus rapide en chasse

                e->vx = (dx / min_dist) * speed;
                e->vy = (dy / min_dist) * speed;
                e->angle = atan2(e->vy, e->vx);

                // Manger la proie si elle est dans la zone d'ingestion (30 px)
                if (min_dist < 30.0) {
                    nearest_prey->active = FALSE;
                    sound_play(SOUND_BITE);
                    if (b->on_entity_eaten) {
                        b->on_entity_eaten(nearest_prey, nearest_prey->score_value, b->callback_data);
                    }
                }
            } else {
                // Errances aléatoires (STATE_SWIMMING)
                e->state = STATE_SWIMMING;
                if (e->vx == 0.0f && e->vy == 0.0f) {
                    float a = (float)(rand() % 628) / 100.0f;
                    e->vx = cosf(a) * base_speed;
                    e->vy = sinf(a) * base_speed;
                    e->angle = a;
                }
                // Changement de cap occasionnel (2% de chance par frame)
                if (rand() % 100 == 0) {
                    float a = (float)(rand() % 628) / 100.0f;
                    e->vx = cosf(a) * base_speed;
                    e->vy = sinf(a) * base_speed;
                    e->angle = a;
                }
            }

            e->x += e->vx * dt * 60.0;
            e->y += e->vy * dt * 60.0;

        } else {
            /* === COMPORTEMENT DE LA PROIE (POISSON...) === */
            Entity *nearest_pred = NULL;
            double min_pred_dist = 99999.0;

            // Chercher le prédateur actif le plus proche
            for (int j = 0; j < b->nb_entities; j++) {
                Entity *other = b->entities[j];
                if (!other || !other->active) continue;
                if (!entity_is_predator(other->type)) continue;

                double dx = other->x - e->x;
                double dy = other->y - e->y;
                double dist = sqrt(dx*dx + dy*dy);
                if (dist < min_pred_dist) {
                    min_pred_dist = dist;
                    nearest_pred = other;
                }
            }

            // Si un prédateur est dans la zone de sécurité (120 px), on fuit !
            if (nearest_pred && min_pred_dist < 120.0) {
                e->state = STATE_FLEEING;
                double dx = e->x - nearest_pred->x;
                double dy = e->y - nearest_pred->y;
                double speed = base_speed * 1.8f; // Fuite rapide

                e->vx = (dx / min_pred_dist) * speed;
                e->vy = (dy / min_pred_dist) * speed;
                e->angle = atan2(e->vy, e->vx);

                // Rompre le banc s'il fuit un requin
                e->leader = NULL;

                e->x += e->vx * dt * 60.0;
                e->y += e->vy * dt * 60.0;

            } else {
                // Pas de danger immédiat, vérifier si on fait partie d'un banc
                if (e->leader && e->leader->active) {
                    e->state = STATE_SWIMMING;
                    double dx = e->leader->x - e->x;
                    double dy = e->leader->y - e->y;
                    double dist = sqrt(dx*dx + dy*dy);

                    if (dist > 35.0) {
                        double speed = base_speed * 1.2f;
                        e->vx = (dx / dist) * speed;
                        e->vy = (dy / dist) * speed;
                        e->angle = atan2(e->vy, e->vx);
                        
                        e->x += e->vx * dt * 60.0;
                        e->y += e->vy * dt * 60.0;
                    }
                } else {
                    // Errance autonome simple
                    e->state = STATE_SWIMMING;
                    if (e->vx == 0.0f && e->vy == 0.0f) {
                        float a = (float)(rand() % 628) / 100.0f;
                        e->vx = cosf(a) * base_speed;
                        e->vy = sinf(a) * base_speed;
                        e->angle = a;
                    }
                    if (rand() % 100 == 0) {
                        float a = (float)(rand() % 628) / 100.0f;
                        e->vx = cosf(a) * base_speed;
                        e->vy = sinf(a) * base_speed;
                        e->angle = a;
                    }

                    e->x += e->vx * dt * 60.0;
                    e->y += e->vy * dt * 60.0;
                }
            }
        }

        /* Rebond sur les limites de la fenêtre */
        if (e->x < 20) { e->x = 20; e->vx = -e->vx; e->angle = atan2(e->vy, e->vx); }
        if (e->x > b->width - 20) { e->x = b->width - 20; e->vx = -e->vx; e->angle = atan2(e->vy, e->vx); }
        if (e->y < 20) { e->y = 20; e->vy = -e->vy; e->angle = atan2(e->vy, e->vx); }
        if (e->y > b->height - 80) { e->y = b->height - 80; e->vy = -e->vy; e->angle = atan2(e->vy, e->vx); }
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

    if (b->mode == BASSIN_MODE_ACCUEIL) {
        draw_ocean_background_ext(cr, w, h, b->time, r1, g1, b1_c, r2, g2, b2_c, 0.35);
    } else {
        draw_ocean_background(cr, w, h, b->time, r1, g1, b1_c, r2, g2, b2_c);
    }
    draw_water_rays(cr, w, h, b->time);
    draw_sand_bottom(cr, w, h);

    /* Bulles */
    for (int i = 0; i < b->nb_bubbles; i++) {
        double wave_x = b->bubbles_x[i] + sin(b->bubbles_phase[i]) * 8.0;
        draw_entity_sprite(cr, FISH_BUBBLE, wave_x, b->bubbles_y[i], b->bubbles_r[i] / 10.0, 0);
    }

    /* Entités */
    for (int i = 0; i < b->nb_entities; i++) {
        Entity *e = b->entities[i];
        if (!e || !e->active) continue;

        double er, eg, eb_c;
        entity_color_to_rgb(e->color, &er, &eg, &eb_c);
        float sz = entity_get_config(e->type)->base_size * e->size_mult;

        /* Rendu du sprite de l'entité */
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
                draw_entity_sprite(cr, OBJ_ROCK, e->x, e->y, sz * 2.0, 0);
                break;
            case OBJ_ALGAE:
                draw_entity_sprite(cr, OBJ_ALGAE, e->x, e->y, sz * 1.5, sin(b->time * 1.5) * 0.1);
                break;
            case OBJ_CORAL:
                draw_coral(cr, e->x, e->y, 0, er, eg, eb_c);
                break;
            default:
                if (e->alpha < 0.99)
                    cairo_push_group(cr);
                
                draw_entity_sprite(cr, e->type, e->x, e->y, sz, e->angle);
                
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

        /* AFFICHAGE DEBUG F5 : Dessiner les zones d'espace et vecteur vitesse */
        extern gboolean g_debug_mode;
        if (g_debug_mode) {
            cairo_save(cr);
            
            // Si c'est un prédateur, on dessine sa zone de détection (180px) et de repas (30px)
            if (entity_is_predator(e->type)) {
                // Zone de détection en rouge transparent
                cairo_set_source_rgba(cr, 0.9, 0.2, 0.2, 0.1);
                cairo_arc(cr, e->x, e->y, 180.0, 0, 2.0 * G_PI);
                cairo_fill(cr);
                
                // Bordure de détection pointillée
                cairo_set_source_rgba(cr, 0.9, 0.2, 0.2, 0.4);
                cairo_set_line_width(cr, 1.5);
                double dashed[] = {4.0, 4.0};
                cairo_set_dash(cr, dashed, 2, 0.0);
                cairo_arc(cr, e->x, e->y, 180.0, 0, 2.0 * G_PI);
                cairo_stroke(cr);

                // Zone d'attaque en rouge vif
                cairo_set_source_rgba(cr, 0.9, 0.1, 0.1, 0.25);
                cairo_arc(cr, e->x, e->y, 30.0, 0, 2.0 * G_PI);
                cairo_fill(cr);
            } 
            // Si c'est un poisson normal, on dessine son espace de sécurité (120px)
            else if (!entity_is_object(e->type)) {
                // Zone de sécurité en bleu transparent
                cairo_set_source_rgba(cr, 0.2, 0.6, 0.9, 0.1);
                cairo_arc(cr, e->x, e->y, 120.0, 0, 2.0 * G_PI);
                cairo_fill(cr);

                // Bordure de sécurité pointillée
                cairo_set_source_rgba(cr, 0.2, 0.6, 0.9, 0.4);
                cairo_set_line_width(cr, 1.5);
                double dashed[] = {4.0, 4.0};
                cairo_set_dash(cr, dashed, 2, 0.0);
                cairo_arc(cr, e->x, e->y, 120.0, 0, 2.0 * G_PI);
                cairo_stroke(cr);
            }

            // Dessin du vecteur vitesse (trait jaune)
            if (!entity_is_object(e->type)) {
                cairo_set_source_rgb(cr, 1.0, 0.9, 0.0);
                cairo_set_line_width(cr, 2.0);
                cairo_move_to(cr, e->x, e->y);
                cairo_line_to(cr, e->x + e->vx * 15.0, e->y + e->vy * 15.0);
                cairo_stroke(cr);
            }

            cairo_restore(cr);
        }
    }

    /* Vignette pour le mode prédateur */
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