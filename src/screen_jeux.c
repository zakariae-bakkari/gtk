#include "screen_jeux.h"
#include "bassin.h"
#include "draw.h"
#include <math.h>
#include <stdio.h>

extern void nav_to_accueil(void);

/* ════════════════════════════════════════════════════
 *  MODE PRÉDATEUR
 * ════════════════════════════════════════════════════ */
static Bassin   *s_pred_bassin  = NULL;
static int       s_pred_score   = 0;
static int       s_pred_timer   = 60;
static int       s_pred_lives   = 3;
static guint     s_pred_tick    = 0;
static GtkWidget *s_pred_overlay_lbl = NULL;

static gboolean pred_game_tick(gpointer data) {
    (void)data;
    s_pred_timer--;

    /* Spawn aléatoire de poissons */
    if (s_pred_bassin && s_pred_bassin->nb_entities < 20) {
        int side = rand() % 4;
        float x, y;
        int w = s_pred_bassin->width, h = s_pred_bassin->height;
        switch (side) {
            case 0: x = 0;   y = rand() % h; break;
            case 1: x = w;   y = rand() % h; break;
            case 2: x = rand() % w; y = 0;   break;
            default:x = rand() % w; y = h;   break;
        }
        EntityType types[] = {FISH_NORMAL, FISH_BLUE, FISH_YELLOW, FISH_CLOWN};
        Entity *e = entity_create(types[rand() % 4], x, y);
        e->state = STATE_SWIMMING;
        e->vx = (w / 2.0f - x) / 200.0f;
        e->vy = (h / 2.0f - y) / 200.0f;
        e->angle = atan2(e->vy, e->vx);
        bassin_add_entity(s_pred_bassin, e);
    }

    if (s_pred_timer <= 0) {
        g_source_remove(s_pred_tick);
        s_pred_tick = 0;
        /* Afficher game over */
        if (s_pred_overlay_lbl) {
            char buf[64];
            snprintf(buf, sizeof(buf), "GAME OVER — Score : %d", s_pred_score);
            gtk_label_set_text(GTK_LABEL(s_pred_overlay_lbl), buf);
        }
    }
    return G_SOURCE_CONTINUE;
}

static void pred_start_game(GtkButton *btn, gpointer data) {
    (void)btn; (void)data;
    s_pred_score = 0;
    s_pred_timer = 60;
    s_pred_lives = 3;
    bassin_clear(s_pred_bassin);
    if (s_pred_tick) g_source_remove(s_pred_tick);
    s_pred_tick = g_timeout_add(1000, pred_game_tick, NULL);
    if (s_pred_overlay_lbl)
        gtk_label_set_text(GTK_LABEL(s_pred_overlay_lbl), "");
}

GtkWidget *screen_predateur_create(void) {
    s_pred_bassin = bassin_create(BASSIN_MODE_PREDATEUR);
    bassin_start(s_pred_bassin);

    GtkWidget *overlay = gtk_overlay_new();
    gtk_overlay_set_child(GTK_OVERLAY(overlay), bassin_get_widget(s_pred_bassin));

    /* HUD */
    GtkWidget *hud = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(hud, GTK_ALIGN_FILL);
    gtk_widget_set_valign(hud, GTK_ALIGN_START);

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p,
            "box { background: rgba(0,0,0,0.5); padding: 10px; }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(top_bar),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }

    s_pred_overlay_lbl = gtk_label_new("");
    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p,
            "label { color: #F1C40F; font-size: 20px; font-weight: bold; }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(s_pred_overlay_lbl),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }

    GtkWidget *btn_start = gtk_button_new_with_label("▶ Démarrer");
    GtkWidget *btn_back  = gtk_button_new_with_label("← Menu");
    g_signal_connect(btn_start, "clicked", G_CALLBACK(pred_start_game), NULL);
    g_signal_connect(btn_back,  "clicked", G_CALLBACK(nav_to_accueil),  NULL);

    gtk_box_append(GTK_BOX(top_bar), btn_start);
    gtk_box_append(GTK_BOX(top_bar), btn_back);
    gtk_box_append(GTK_BOX(top_bar), s_pred_overlay_lbl);
    gtk_box_append(GTK_BOX(hud), top_bar);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), hud);
    return overlay;
}

/* ════════════════════════════════════════════════════
 *  MODE SURVIE
 * ════════════════════════════════════════════════════ */
static Bassin   *s_surv_bassin  = NULL;
static int       s_surv_time    = 0;
static int       s_surv_wave    = 1;
static guint     s_surv_tick    = 0;
static GtkWidget *s_surv_lbl   = NULL;

static gboolean surv_game_tick(gpointer data) {
    (void)data;
    s_surv_time++;

    /* Spawn d'un requin par vague */
    if (s_surv_time % 10 == 0 && s_surv_bassin) {
        s_surv_wave = s_surv_time / 10 + 1;
        int w = s_surv_bassin->width, h = s_surv_bassin->height;
        Entity *shark = entity_create(SHARK_WHITE, rand() % w, 0);
        shark->state = STATE_CHASING;
        shark->vx = (w / 2.0f - shark->x) / 100.0f;
        shark->vy = h / 100.0f;
        shark->angle = atan2(shark->vy, shark->vx);
        bassin_add_entity(s_surv_bassin, shark);
    }

    if (s_surv_lbl) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Survie : %ds | Vague %d", s_surv_time, s_surv_wave);
        gtk_label_set_text(GTK_LABEL(s_surv_lbl), buf);
    }
    return G_SOURCE_CONTINUE;
}

static void surv_start_game(GtkButton *btn, gpointer data) {
    (void)btn; (void)data;
    s_surv_time = 0;
    s_surv_wave = 1;
    bassin_clear(s_surv_bassin);

    /* Créer le poisson joueur */
    if (s_surv_bassin) {
        Entity *player = entity_create(FISH_NORMAL,
            s_surv_bassin->width / 2.0f, s_surv_bassin->height / 2.0f);
        player->color = COLOR_ORANGE;
        player->is_leader = TRUE;
        player->state = STATE_IDLE;
        bassin_add_entity(s_surv_bassin, player);
        s_surv_bassin->player = player;
    }

    if (s_surv_tick) g_source_remove(s_surv_tick);
    s_surv_tick = g_timeout_add(1000, surv_game_tick, NULL);
}

GtkWidget *screen_survie_create(void) {
    s_surv_bassin = bassin_create(BASSIN_MODE_SURVIE);
    bassin_start(s_surv_bassin);

    GtkWidget *overlay = gtk_overlay_new();
    gtk_overlay_set_child(GTK_OVERLAY(overlay), bassin_get_widget(s_surv_bassin));

    GtkWidget *hud = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(hud, GTK_ALIGN_FILL);
    gtk_widget_set_valign(hud, GTK_ALIGN_START);

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p,
            "box { background: rgba(0,0,0,0.5); padding: 10px; }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(top_bar),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }

    s_surv_lbl = gtk_label_new("Survie : 0s | Vague 1");
    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p,
            "label { color: white; font-size: 18px; font-weight: bold; }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(s_surv_lbl),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }

    GtkWidget *btn_start = gtk_button_new_with_label("▶ Démarrer");
    GtkWidget *btn_back  = gtk_button_new_with_label("← Menu");
    g_signal_connect(btn_start, "clicked", G_CALLBACK(surv_start_game), NULL);
    g_signal_connect(btn_back,  "clicked", G_CALLBACK(nav_to_accueil),  NULL);

    gtk_box_append(GTK_BOX(top_bar), btn_start);
    gtk_box_append(GTK_BOX(top_bar), btn_back);
    gtk_box_append(GTK_BOX(top_bar), s_surv_lbl);
    gtk_box_append(GTK_BOX(hud), top_bar);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), hud);
    return overlay;
}