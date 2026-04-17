#ifndef DRAW_H
#define DRAW_H

#include <cairo.h>
#include <gtk/gtk.h>
#include "entities.h"

/* ── Fond & décors ─────────────────────────────────── */
void draw_ocean_background(cairo_t *cr, int w, int h, double time,
                            double r1, double g1, double b1,   /* couleur haut */
                            double r2, double g2, double b2);  /* couleur bas  */
void draw_water_rays      (cairo_t *cr, int w, int h, double time);
void draw_sand_bottom     (cairo_t *cr, int w, int h);
void draw_vignette        (cairo_t *cr, int w, int h, double alpha);

/* ── Décor vivant ───────────────────────────────────── */
void draw_bubble  (cairo_t *cr, double x, double y, double radius, double alpha);
void draw_algae   (cairo_t *cr, double x, double y, double height, double sway);
void draw_coral   (cairo_t *cr, double x, double y, int type, double r, double g, double b);
void draw_rock    (cairo_t *cr, double x, double y, double size);

/* ── Entités ─────────────────────────────────────────── */
void draw_fish    (cairo_t *cr, double x, double y, double size,
                   double r, double g, double b, double angle, double anim_phase);
void draw_shark   (cairo_t *cr, double x, double y, double size,
                   double angle, double anim_phase);
void draw_turtle  (cairo_t *cr, double x, double y, double size, double angle);
void draw_jellyfish(cairo_t *cr, double x, double y, double size, double anim_phase);
void draw_dolphin (cairo_t *cr, double x, double y, double size, double angle);

/* ── HUD ─────────────────────────────────────────────── */
void draw_hud_score(cairo_t *cr, int x, int y, int score);
void draw_hud_timer(cairo_t *cr, int x, int y, int seconds, gboolean urgent);
void draw_hud_lives(cairo_t *cr, int x, int y, int lives);

/* ── Effets ──────────────────────────────────────────── */
void draw_blood_splat (cairo_t *cr, double x, double y, double alpha);
void draw_glow        (cairo_t *cr, double x, double y, double radius,
                       double r, double g, double b, double alpha);
void draw_powerup     (cairo_t *cr, double x, double y, int type, double anim_phase);

/* ── Couleur depuis EntityColor ──────────────────────── */
void entity_color_to_rgb(EntityColor color, double *r, double *g, double *b);

#endif /* DRAW_H */