#include "draw.h"
#include <math.h>

/* ── Utilitaires ─────────────────────────────────── */
void entity_color_to_rgb(EntityColor color, double *r, double *g, double *b) {
    switch (color) {
        case COLOR_RED:    *r=0.9; *g=0.2; *b=0.2; break;
        case COLOR_BLUE:   *r=0.2; *g=0.5; *b=0.9; break;
        case COLOR_YELLOW: *r=1.0; *g=0.8; *b=0.1; break;
        case COLOR_GREEN:  *r=0.2; *g=0.8; *b=0.3; break;
        case COLOR_ORANGE: *r=0.95;*g=0.5; *b=0.1; break;
        case COLOR_WHITE:  *r=1.0; *g=1.0; *b=1.0; break;
        case COLOR_PURPLE: *r=0.6; *g=0.2; *b=0.9; break;
        case COLOR_CYAN:   *r=0.1; *g=0.9; *b=0.9; break;
        default:           *r=0.5; *g=0.5; *b=0.5; break;
    }
}

/* ── Fond océan ──────────────────────────────────── */
void draw_ocean_background(cairo_t *cr, int w, int h, double time,
                            double r1, double g1, double b1,
                            double r2, double g2, double b2) {
    cairo_pattern_t *pat = cairo_pattern_create_linear(0, 0, 0, h);
    cairo_pattern_add_color_stop_rgb(pat, 0.0, r1, g1, b1);
    cairo_pattern_add_color_stop_rgb(pat, 1.0, r2, g2, b2);
    cairo_set_source(cr, pat);
    cairo_rectangle(cr, 0, 0, w, h);
    cairo_fill(cr);
    cairo_pattern_destroy(pat);
    (void)time;
}

void draw_water_rays(cairo_t *cr, int w, int h, double time) {
    cairo_save(cr);
    for (int i = 0; i < 5; i++) {
        double x = (w / 5.0) * i + sin(time * 0.3 + i) * 30.0;
        double alpha = 0.04 + 0.02 * sin(time * 0.5 + i);
        cairo_pattern_t *pat = cairo_pattern_create_linear(x, 0, x + 80, h);
        cairo_pattern_add_color_stop_rgba(pat, 0.0, 1, 1, 1, alpha);
        cairo_pattern_add_color_stop_rgba(pat, 1.0, 1, 1, 1, 0.0);
        cairo_set_source(cr, pat);
        cairo_rectangle(cr, x - 40, 0, 120, h);
        cairo_fill(cr);
        cairo_pattern_destroy(pat);
    }
    cairo_restore(cr);
}

void draw_sand_bottom(cairo_t *cr, int w, int h) {
    cairo_save(cr);
    cairo_pattern_t *pat = cairo_pattern_create_linear(0, h - 60, 0, h);
    cairo_pattern_add_color_stop_rgb(pat, 0.0, 0.76, 0.65, 0.40);
    cairo_pattern_add_color_stop_rgb(pat, 1.0, 0.65, 0.55, 0.30);
    cairo_set_source(cr, pat);
    cairo_rectangle(cr, 0, h - 60, w, 60);
    cairo_fill(cr);
    cairo_pattern_destroy(pat);
    /* petits points de sable */
    cairo_set_source_rgba(cr, 0.85, 0.75, 0.5, 0.5);
    for (int i = 0; i < 80; i++) {
        double sx = (w / 80.0) * i + ((i * 37) % 20 - 10);
        double sy = h - 50 + (i * 13) % 40;
        cairo_arc(cr, sx, sy, 1.5, 0, 2 * G_PI);
        cairo_fill(cr);
    }
    cairo_restore(cr);
}

void draw_vignette(cairo_t *cr, int w, int h, double alpha) {
    cairo_pattern_t *pat = cairo_pattern_create_radial(
        w / 2.0, h / 2.0, h * 0.3,
        w / 2.0, h / 2.0, h * 0.9);
    cairo_pattern_add_color_stop_rgba(pat, 0.0, 0, 0, 0, 0.0);
    cairo_pattern_add_color_stop_rgba(pat, 1.0, 0, 0, 0, alpha);
    cairo_set_source(cr, pat);
    cairo_rectangle(cr, 0, 0, w, h);
    cairo_fill(cr);
    cairo_pattern_destroy(pat);
}

/* ── Bulle ───────────────────────────────────────── */
void draw_bubble(cairo_t *cr, double x, double y, double radius, double alpha) {
    cairo_save(cr);
    cairo_set_source_rgba(cr, 0.7, 0.9, 1.0, alpha * 0.4);
    cairo_arc(cr, x, y, radius, 0, 2 * G_PI);
    cairo_fill(cr);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, alpha * 0.7);
    cairo_set_line_width(cr, 1.0);
    cairo_arc(cr, x, y, radius, 0, 2 * G_PI);
    cairo_stroke(cr);
    /* reflet */
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, alpha * 0.5);
    cairo_arc(cr, x - radius * 0.3, y - radius * 0.3, radius * 0.2, 0, 2 * G_PI);
    cairo_fill(cr);
    cairo_restore(cr);
}

/* ── Algue ───────────────────────────────────────── */
void draw_algae(cairo_t *cr, double x, double y, double height, double sway) {
    cairo_save(cr);
    cairo_set_source_rgb(cr, 0.1, 0.6, 0.3);
    cairo_set_line_width(cr, 3.0);
    cairo_move_to(cr, x, y);
    cairo_curve_to(cr,
        x + sway * 20, y - height * 0.33,
        x - sway * 20, y - height * 0.66,
        x + sway * 10, y - height);
    cairo_stroke(cr);
    cairo_restore(cr);
}

/* ── Corail ──────────────────────────────────────── */
void draw_coral(cairo_t *cr, double x, double y, int type, double r, double g, double b) {
    cairo_save(cr);
    cairo_set_source_rgb(cr, r, g, b);
    if (type == 0) {
        /* corail en Y */
        cairo_set_line_width(cr, 4.0);
        cairo_move_to(cr, x, y);
        cairo_line_to(cr, x, y - 30);
        cairo_stroke(cr);
        cairo_move_to(cr, x, y - 20);
        cairo_line_to(cr, x - 15, y - 40);
        cairo_stroke(cr);
        cairo_move_to(cr, x, y - 20);
        cairo_line_to(cr, x + 15, y - 40);
        cairo_stroke(cr);
    } else {
        /* corail rond */
        cairo_arc(cr, x, y - 15, 12, 0, 2 * G_PI);
        cairo_fill(cr);
        cairo_set_source_rgb(cr, r * 0.7, g * 0.7, b * 0.7);
        cairo_arc(cr, x, y - 15, 12, 0, 2 * G_PI);
        cairo_set_line_width(cr, 1.5);
        cairo_stroke(cr);
    }
    cairo_restore(cr);
}

/* ── Rocher ──────────────────────────────────────── */
void draw_rock(cairo_t *cr, double x, double y, double size) {
    cairo_save(cr);
    cairo_set_source_rgb(cr, 0.45, 0.45, 0.45);
    cairo_move_to(cr, x - size * 0.8, y);
    cairo_curve_to(cr, x - size, y - size * 0.5, x - size * 0.3, y - size, x, y - size * 0.9);
    cairo_curve_to(cr, x + size * 0.3, y - size, x + size, y - size * 0.5, x + size * 0.8, y);
    cairo_close_path(cr);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0.6, 0.6, 0.6);
    cairo_move_to(cr, x - size * 0.3, y - size * 0.6);
    cairo_curve_to(cr, x - size * 0.1, y - size * 0.9, x + size * 0.2, y - size * 0.7, x + size * 0.3, y - size * 0.5);
    cairo_set_line_width(cr, 1.5);
    cairo_stroke(cr);
    cairo_restore(cr);
}

/* ── Poisson ─────────────────────────────────────── */
void draw_fish(cairo_t *cr, double x, double y, double size,
               double r, double g, double b, double angle, double anim_phase) {
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_rotate(cr, angle);

    double tail_wave = sin(anim_phase * 4.0) * 0.2;

    /* corps */
    cairo_set_source_rgb(cr, r, g, b);
    cairo_save(cr);
    cairo_scale(cr, size * 1.5, size);
    cairo_arc(cr, 0, 0, 10, 0, 2 * G_PI);
    cairo_restore(cr);
    cairo_fill(cr);

    /* queue */
    cairo_set_source_rgb(cr, r * 0.8, g * 0.8, b * 0.8);
    cairo_move_to(cr, -size * 12, 0);
    cairo_line_to(cr, -size * 20, -size * 8 + tail_wave * size * 10);
    cairo_line_to(cr, -size * 20,  size * 8 + tail_wave * size * 10);
    cairo_close_path(cr);
    cairo_fill(cr);

    /* œil */
    cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
    cairo_arc(cr, size * 6, -size * 2, size * 1.5, 0, 2 * G_PI);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_arc(cr, size * 6.5, -size * 2.5, size * 0.5, 0, 2 * G_PI);
    cairo_fill(cr);

    /* nageoire dorsale */
    cairo_set_source_rgb(cr, r * 0.9, g * 0.9, b * 0.9);
    cairo_move_to(cr, 0, -size * 8);
    cairo_curve_to(cr, size * 3, -size * 16, size * 8, -size * 12, size * 5, -size * 8);
    cairo_close_path(cr);
    cairo_fill(cr);

    cairo_restore(cr);
}

/* ── Requin ──────────────────────────────────────── */
void draw_shark(cairo_t *cr, double x, double y, double size,
                double angle, double anim_phase) {
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_rotate(cr, angle);

    double tail_wave = sin(anim_phase * 3.0) * 0.15;

    /* corps gris */
    cairo_set_source_rgb(cr, 0.55, 0.55, 0.6);
    cairo_save(cr);
    cairo_scale(cr, size * 3.0, size);
    cairo_arc(cr, 0, 0, 10, 0, 2 * G_PI);
    cairo_restore(cr);
    cairo_fill(cr);

    /* ventre blanc */
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_save(cr);
    cairo_scale(cr, size * 2.5, size * 0.5);
    cairo_arc(cr, 2, 4, 9, 0, 2 * G_PI);
    cairo_restore(cr);
    cairo_fill(cr);

    /* aileron dorsal */
    cairo_set_source_rgb(cr, 0.45, 0.45, 0.5);
    cairo_move_to(cr, size * 5, -size * 8);
    cairo_line_to(cr, size * 10, -size * 20);
    cairo_line_to(cr, size * 18, -size * 8);
    cairo_close_path(cr);
    cairo_fill(cr);

    /* queue */
    cairo_move_to(cr, -size * 25, 0);
    cairo_line_to(cr, -size * 38, -size * 14 + tail_wave * size * 15);
    cairo_line_to(cr, -size * 38,  size * 14 + tail_wave * size * 15);
    cairo_close_path(cr);
    cairo_fill(cr);

    /* œil */
    cairo_set_source_rgb(cr, 0.05, 0.05, 0.05);
    cairo_arc(cr, size * 18, -size * 3, size * 2, 0, 2 * G_PI);
    cairo_fill(cr);

    cairo_restore(cr);
}

/* ── Tortue ──────────────────────────────────────── */
void draw_turtle(cairo_t *cr, double x, double y, double size, double angle) {
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_rotate(cr, angle);

    /* carapace */
    cairo_set_source_rgb(cr, 0.25, 0.55, 0.25);
    cairo_save(cr);
    cairo_scale(cr, size * 1.2, size);
    cairo_arc(cr, 0, 0, 14, 0, 2 * G_PI);
    cairo_restore(cr);
    cairo_fill(cr);

    /* motif carapace */
    cairo_set_source_rgb(cr, 0.2, 0.45, 0.2);
    cairo_set_line_width(cr, 1.5);
    cairo_move_to(cr, 0, -size * 10);
    cairo_line_to(cr, 0, size * 10);
    cairo_stroke(cr);
    cairo_move_to(cr, -size * 8, 0);
    cairo_line_to(cr, size * 8, 0);
    cairo_stroke(cr);

    /* tête */
    cairo_set_source_rgb(cr, 0.3, 0.6, 0.3);
    cairo_arc(cr, size * 14, 0, size * 4, 0, 2 * G_PI);
    cairo_fill(cr);

    /* pattes */
    for (int i = 0; i < 4; i++) {
        double px = (i < 2) ? size * 8 : -size * 8;
        double py = (i % 2 == 0) ? size * 10 : -size * 10;
        cairo_arc(cr, px, py, size * 3, 0, 2 * G_PI);
        cairo_fill(cr);
    }

    cairo_restore(cr);
}

/* ── Méduse ──────────────────────────────────────── */
void draw_jellyfish(cairo_t *cr, double x, double y, double size, double anim_phase) {
    cairo_save(cr);
    double pulse = 1.0 + 0.1 * sin(anim_phase * 2.0);
    cairo_translate(cr, x, y);

    /* cloche */
    cairo_set_source_rgba(cr, 0.8, 0.4, 0.9, 0.6);
    cairo_save(cr);
    cairo_scale(cr, size * pulse, size * 0.7 * pulse);
    cairo_arc(cr, 0, 0, 12, G_PI, 2 * G_PI);
    cairo_close_path(cr);
    cairo_restore(cr);
    cairo_fill(cr);

    /* tentacules */
    cairo_set_source_rgba(cr, 0.8, 0.4, 0.9, 0.4);
    cairo_set_line_width(cr, 1.5);
    for (int i = -2; i <= 2; i++) {
        double tx = i * size * 3;
        double wave = sin(anim_phase * 3.0 + i) * size * 4;
        cairo_move_to(cr, tx, 0);
        cairo_curve_to(cr, tx + wave, size * 8, tx - wave, size * 16, tx, size * 20);
        cairo_stroke(cr);
    }

    cairo_restore(cr);
}

/* ── Dauphin ─────────────────────────────────────── */
void draw_dolphin(cairo_t *cr, double x, double y, double size, double angle) {
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_rotate(cr, angle);

    cairo_set_source_rgb(cr, 0.4, 0.6, 0.8);
    cairo_save(cr);
    cairo_scale(cr, size * 2.5, size);
    cairo_arc(cr, 0, 0, 12, 0, 2 * G_PI);
    cairo_restore(cr);
    cairo_fill(cr);

    /* aileron */
    cairo_set_source_rgb(cr, 0.35, 0.55, 0.75);
    cairo_move_to(cr, size * 4, -size * 10);
    cairo_line_to(cr, size * 8, -size * 22);
    cairo_line_to(cr, size * 16, -size * 10);
    cairo_close_path(cr);
    cairo_fill(cr);

    cairo_restore(cr);
}

/* ── HUD ─────────────────────────────────────────── */
void draw_hud_score(cairo_t *cr, int x, int y, int score) {
    cairo_save(cr);
    cairo_select_font_face(cr, "Monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 28);
    cairo_set_source_rgba(cr, 0, 0, 0, 0.5);
    cairo_move_to(cr, x + 2, y + 2);
    char buf[32];
    snprintf(buf, sizeof(buf), "Score: %d", score);
    cairo_show_text(cr, buf);
    cairo_set_source_rgb(cr, 0.95, 0.76, 0.05);
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, buf);
    cairo_restore(cr);
}

void draw_hud_timer(cairo_t *cr, int x, int y, int seconds, gboolean urgent) {
    cairo_save(cr);
    cairo_select_font_face(cr, "Monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 28);
    if (urgent)
        cairo_set_source_rgb(cr, 0.9, 0.1, 0.1);
    else
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d", seconds);
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, buf);
    cairo_restore(cr);
}

void draw_hud_lives(cairo_t *cr, int x, int y, int lives) {
    for (int i = 0; i < lives; i++) {
        draw_shark(cr, x + i * 30, y, 0.5, 0, 0);
    }
}

/* ── Effets ──────────────────────────────────────── */
void draw_blood_splat(cairo_t *cr, double x, double y, double alpha) {
    cairo_save(cr);
    cairo_set_source_rgba(cr, 0.8, 0.0, 0.0, alpha * 0.6);
    for (int i = 0; i < 6; i++) {
        double a = (G_PI * 2.0 / 6.0) * i;
        double r = 8.0 + (i * 5) % 10;
        cairo_arc(cr, x + cos(a) * r, y + sin(a) * r, 4 + (i % 3), 0, 2 * G_PI);
        cairo_fill(cr);
    }
    cairo_restore(cr);
}

void draw_glow(cairo_t *cr, double x, double y, double radius,
               double r, double g, double b, double alpha) {
    cairo_pattern_t *pat = cairo_pattern_create_radial(x, y, 0, x, y, radius);
    cairo_pattern_add_color_stop_rgba(pat, 0.0, r, g, b, alpha);
    cairo_pattern_add_color_stop_rgba(pat, 1.0, r, g, b, 0.0);
    cairo_set_source(cr, pat);
    cairo_arc(cr, x, y, radius, 0, 2 * G_PI);
    cairo_fill(cr);
    cairo_pattern_destroy(pat);
}

void draw_powerup(cairo_t *cr, double x, double y, int type, double anim_phase) {
    double pulse = 1.0 + 0.15 * sin(anim_phase * 4.0);
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_scale(cr, pulse, pulse);
    switch (type) {
        case 0: /* boost vitesse — éclair jaune */
            cairo_set_source_rgb(cr, 1.0, 0.9, 0.1);
            cairo_move_to(cr, 5, -15);
            cairo_line_to(cr, -3, 0);
            cairo_line_to(cr, 3, 0);
            cairo_line_to(cr, -5, 15);
            cairo_line_to(cr, 5, 2);
            cairo_line_to(cr, -1, 2);
            cairo_close_path(cr);
            cairo_fill(cr);
            break;
        case 1: /* invincibilité — étoile blanche */
            cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
            for (int i = 0; i < 5; i++) {
                double a = -G_PI / 2.0 + (2 * G_PI / 5.0) * i;
                double ax = cos(a) * 12, ay = sin(a) * 12;
                double bx = cos(a + G_PI / 5.0) * 5, by = sin(a + G_PI / 5.0) * 5;
                if (i == 0) cairo_move_to(cr, ax, ay); else cairo_line_to(cr, ax, ay);
                cairo_line_to(cr, bx, by);
            }
            cairo_close_path(cr);
            cairo_fill(cr);
            break;
        case 2: /* ralentir — spirale bleue */
            cairo_set_source_rgb(cr, 0.2, 0.5, 1.0);
            cairo_set_line_width(cr, 3.0);
            cairo_arc(cr, 0, 0, 10, 0, 2 * G_PI);
            cairo_stroke(cr);
            break;
    }
    cairo_restore(cr);
}