#include "screen_jeux.h"
#include "bassin.h"
#include "draw.h"
#include "sound.h"
#include "entities.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../widgets/headers/common.h"
#include "../widgets/headers/champ_select.h"
#include "../widgets/headers/dialog.h"

extern void nav_to_accueil(void);
extern GtkWidget *g_window;

/* Variables globales de contrôle clavier temporaires */
static Entity *s_active_player = NULL;
static GtkEventController *s_game_key_ctrl = NULL;

/* ════════════════════════════════════════════════════
 *  GESTIONNAIRE CLAVIER ET NAVIGATION
 * ════════════════════════════════════════════════════ */
static gboolean game_key_pressed(GtkEventControllerKey *ctrl, guint keyval, guint keycode, GdkModifierType state, gpointer data) {
    (void)ctrl; (void)keycode; (void)state; (void)data;
    Entity *p = s_active_player;
    if (!p || !p->active) return FALSE;

    float speed = entity_get_config(p->type)->base_speed * p->speed_mult * 1.5f;
    gboolean handled = FALSE;

    if (keyval == GDK_KEY_Up || keyval == GDK_KEY_w || keyval == GDK_KEY_z) {
        p->vy = -speed;
        handled = TRUE;
    } else if (keyval == GDK_KEY_Down || keyval == GDK_KEY_s) {
        p->vy = speed;
        handled = TRUE;
    } else if (keyval == GDK_KEY_Left || keyval == GDK_KEY_a || keyval == GDK_KEY_q) {
        p->vx = -speed;
        handled = TRUE;
    } else if (keyval == GDK_KEY_Right || keyval == GDK_KEY_d) {
        p->vx = speed;
        handled = TRUE;
    }

    if (handled) {
        p->angle = atan2(p->vy, p->vx);
        p->x += p->vx * 1.5f;
        p->y += p->vy * 1.5f;
        return TRUE;
    }
    return FALSE;
}

static gboolean game_key_released(GtkEventControllerKey *ctrl, guint keyval, guint keycode, GdkModifierType state, gpointer data) {
    (void)ctrl; (void)keycode; (void)state; (void)data;
    Entity *p = s_active_player;
    if (!p || !p->active) return FALSE;

    if (keyval == GDK_KEY_Up || keyval == GDK_KEY_w || keyval == GDK_KEY_z ||
        keyval == GDK_KEY_Down || keyval == GDK_KEY_s) {
        p->vy = 0;
        return TRUE;
    }
    if (keyval == GDK_KEY_Left || keyval == GDK_KEY_a || keyval == GDK_KEY_q ||
        keyval == GDK_KEY_Right || keyval == GDK_KEY_d) {
        p->vx = 0;
        return TRUE;
    }
    return FALSE;
}

static void setup_keyboard_listener(void) {
    if (s_game_key_ctrl) return;
    s_game_key_ctrl = gtk_event_controller_key_new();
    g_signal_connect(s_game_key_ctrl, "key-pressed", G_CALLBACK(game_key_pressed), NULL);
    g_signal_connect(s_game_key_ctrl, "key-released", G_CALLBACK(game_key_released), NULL);
    gtk_widget_add_controller(GTK_WIDGET(g_window), s_game_key_ctrl);
}

static void clean_keyboard_listener(void) {
    if (s_game_key_ctrl && g_window) {
        gtk_widget_remove_controller(GTK_WIDGET(g_window), s_game_key_ctrl);
        s_game_key_ctrl = NULL;
    }
    s_active_player = NULL;
}

/* ════════════════════════════════════════════════════
 *  CONTROLE PAR LA SOURIS (SUIVI DU CURSEUR)
 * ════════════════════════════════════════════════════ */
static void on_bassin_motion(GtkEventControllerMotion *ctrl, double x, double y, gpointer data) {
    (void)ctrl;
    Bassin *b = (Bassin *)data;
    if (!b || !b->player || !b->player->active) return;

    double dx = x - b->player->x;
    double dy = y - b->player->y;
    double dist = sqrt(dx*dx + dy*dy);

    if (dist > 20.0) {
        float speed = entity_get_config(b->player->type)->base_speed * b->player->speed_mult * 1.3f;
        b->player->vx = (dx / dist) * speed;
        b->player->vy = (dy / dist) * speed;
        b->player->angle = atan2(b->player->vy, b->player->vx);
    } else {
        b->player->vx = 0;
        b->player->vy = 0;
    }
}

/* ════════════════════════════════════════════════════
 *  MODE PRÉDATEUR
 * ════════════════════════════════════════════════════ */
static Bassin   *s_pred_bassin  = NULL;
static int       s_pred_score   = 0;
static int       s_pred_timer   = 60;
static guint     s_pred_tick    = 0;
static GtkWidget *s_pred_overlay_lbl = NULL;
static ChampSelect *s_choice_pred_sel = NULL;

static void update_pred_hud_label(void) {
    if (!s_pred_overlay_lbl) return;
    char buf[128];
    snprintf(buf, sizeof(buf), "⏱️ Temps restant : %ds  |  🎯 Poissons dévorés : %d/15  |  🦈 Mode prédateur", 
             s_pred_timer, s_pred_score);
    gtk_label_set_text(GTK_LABEL(s_pred_overlay_lbl), buf);
}

static void on_pred_entity_eaten(Entity *e, int score, gpointer data) {
    (void)e; (void)data;
    s_pred_score += 1; // Un poisson mangé
    update_pred_hud_label();

    if (s_pred_score >= 15) {
        if (s_pred_tick) { g_source_remove(s_pred_tick); s_pred_tick = 0; }
        sound_play(SOUND_VICTORY);
        gtk_label_set_text(GTK_LABEL(s_pred_overlay_lbl), "🎉 VICTOIRE ! Vous avez dévoré 15 poissons !");
    }
}

static gboolean pred_game_tick(gpointer data) {
    (void)data;
    s_pred_timer--;
    update_pred_hud_label();

    /* Spawn aléatoire de proies */
    if (s_pred_bassin && s_pred_bassin->nb_entities < 22) {
        int side = rand() % 4;
        float x, y;
        int w = s_pred_bassin->width, h = s_pred_bassin->height;
        switch (side) {
            case 0: x = 0;   y = rand() % h; break;
            case 1: x = w;   y = rand() % h; break;
            case 2: x = rand() % w; y = 0;   break;
            default:x = rand() % w; y = h;   break;
        }
        EntityType types[] = {FISH_NORMAL, FISH_BLUE, FISH_YELLOW, FISH_CLOWN, FISH_BALLOON, JELLYFISH, TURTLE_SEA};
        Entity *e = entity_create(types[rand() % 7], x, y);
        if (e) {
            e->state = STATE_SWIMMING;
            e->vx = (w / 2.0f - x) / 180.0f;
            e->vy = (h / 2.0f - y) / 180.0f;
            e->angle = atan2(e->vy, e->vx);
            bassin_add_entity(s_pred_bassin, e);
        }
    }

    if (s_pred_timer <= 0) {
        if (s_pred_tick) { g_source_remove(s_pred_tick); s_pred_tick = 0; }
        sound_play(SOUND_GAME_OVER);
        char buf[128];
        snprintf(buf, sizeof(buf), "💀 TEMPS ÉCOULÉ ! Score final : %d/15", s_pred_score);
        gtk_label_set_text(GTK_LABEL(s_pred_overlay_lbl), buf);
    }
    return G_SOURCE_CONTINUE;
}

static void pred_initialize_game_with_type(EntityType shark_type) {
    s_pred_score = 0;
    s_pred_timer = 60;
    bassin_clear(s_pred_bassin);

    /* Créer le prédateur du joueur */
    Entity *player = entity_create(shark_type, s_pred_bassin->width / 2.0f, s_pred_bassin->height / 2.0f);
    if (player) {
        player->aggressive = TRUE;
        player->state = STATE_IDLE;
        player->size_mult = 1.3f;
        bassin_add_entity(s_pred_bassin, player);
        s_pred_bassin->player = player;
        s_active_player = player;
    }

    /* Enregistrer le callback de repas */
    s_pred_bassin->on_entity_eaten = on_pred_entity_eaten;
    s_pred_bassin->callback_data = NULL;

    /* Spawn des poissons initiaux */
    for (int i = 0; i < 15; i++) {
        EntityType types[] = {FISH_NORMAL, FISH_BLUE, FISH_YELLOW, FISH_CLOWN, FISH_BALLOON};
        Entity *e = entity_create(types[rand() % 5], (float)(rand() % (s_pred_bassin->width - 100) + 50), 
                                                     (float)(rand() % (s_pred_bassin->height - 150) + 50));
        if (e) {
            e->state = STATE_SWIMMING;
            e->vx = 1.0f * (rand() % 2 == 0 ? 1 : -1);
            e->vy = 0.5f * (rand() % 2 == 0 ? 1 : -1);
            e->angle = atan2(e->vy, e->vx);
            bassin_add_entity(s_pred_bassin, e);
        }
    }

    setup_keyboard_listener();
    sound_play(SOUND_SHARK_ALERT);

    if (s_pred_tick) g_source_remove(s_pred_tick);
    s_pred_tick = g_timeout_add(1000, pred_game_tick, NULL);
    update_pred_hud_label();
}

static void on_choice_pred_reponse(int reponse_id, gpointer user_data) {
    Dialog *dlg = (Dialog *)user_data;
    EntityType chosen_type = SHARK_WHITE;

    if (reponse_id == DIALOG_REPONSE_OUI || reponse_id == DIALOG_REPONSE_OK) {
        int idx = s_choice_pred_sel->selected_index;
        switch (idx) {
            case 0: chosen_type = SHARK_WHITE; break;
            case 1: chosen_type = SHARK_MAKO; break;
            case 2: chosen_type = SHARK_HAMMER; break;
            case 3: chosen_type = SHARK_TIGER; break;
            case 4: chosen_type = ORCA; break;
        }
    }

    g_free(s_choice_pred_sel);
    s_choice_pred_sel = NULL;
    pred_initialize_game_with_type(chosen_type);
}

static void pred_start_game(GtkButton *btn, gpointer data) {
    (void)btn; (void)data;
    
    /* Ouvrir la boîte de dialogue de choix */
    Dialog *dlg = g_new0(Dialog, 1);
    dialog_initialiser(dlg);
    dlg->parent = GTK_WINDOW(g_window);
    dlg->type = DIALOG_INFO;
    dlg->titre = g_strdup("🦈 Sélectionnez votre Prédateur");
    dlg->message = g_strdup("Choisissez votre requin ou orque :");
    dlg->boutons_preset = DIALOG_BOUTONS_OUI_NON;
    dlg->on_reponse = on_choice_pred_reponse;
    dlg->user_data = dlg;

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(content, 15);
    gtk_widget_set_margin_end(content, 15);
    
    s_choice_pred_sel = g_new0(ChampSelect, 1);
    champ_select_initialiser(s_choice_pred_sel);
    s_choice_pred_sel->selected_index = 0;
    champ_select_add_item(s_choice_pred_sel, "🦈 Grand Requin Blanc");
    champ_select_add_item(s_choice_pred_sel, "⚡ Requin Mako ultra-rapide");
    champ_select_add_item(s_choice_pred_sel, "🔨 Requin Marteau");
    champ_select_add_item(s_choice_pred_sel, "🐅 Requin Tigre");
    champ_select_add_item(s_choice_pred_sel, "🐋 Orque Tueuse");

    GtkWidget *w_sel = champ_select_creer(s_choice_pred_sel);
    gtk_box_append(GTK_BOX(content), w_sel);
    dlg->widget_contenu = content;

    dialog_creer(dlg);
    dialog_afficher(dlg);
    g_signal_connect_swapped(dlg->window, "destroy", G_CALLBACK(dialog_free), dlg);
}

static void on_leave_pred(GtkButton *btn, gpointer data) {
    (void)btn; (void)data;
    clean_keyboard_listener();
    if (s_pred_tick) { g_source_remove(s_pred_tick); s_pred_tick = 0; }
    nav_to_accueil();
}

GtkWidget *screen_predateur_create(void) {
    s_pred_bassin = bassin_create(BASSIN_MODE_PREDATEUR);
    bassin_start(s_pred_bassin);

    GtkWidget *overlay = gtk_overlay_new();
    GtkWidget *canvas = bassin_get_widget(s_pred_bassin);
    gtk_overlay_set_child(GTK_OVERLAY(overlay), canvas);

    /* Gestionnaire souris */
    GtkEventController *motion = gtk_event_controller_motion_new();
    g_signal_connect(motion, "motion", G_CALLBACK(on_bassin_motion), (gpointer)s_pred_bassin);
    gtk_widget_add_controller(canvas, motion);

    /* HUD */
    GtkWidget *hud = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(hud, GTK_ALIGN_FILL);
    gtk_widget_set_valign(hud, GTK_ALIGN_START);

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p,
            "box { background: rgba(5,15,30,0.85); padding: 12px; border-bottom: 2px solid rgba(255,255,255,0.08); }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(top_bar),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }

    s_pred_overlay_lbl = gtk_label_new("Cliquez sur Démarrer pour choisir votre prédateur !");
    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p,
            "label { color: #F1C40F; font-size: 18px; font-weight: bold; }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(s_pred_overlay_lbl),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }

    GtkWidget *btn_start = gtk_button_new_with_label("▶ Choisir & Démarrer");
    GtkWidget *btn_back  = gtk_button_new_with_label("← Accueil Menu");
    g_signal_connect(btn_start, "clicked", G_CALLBACK(pred_start_game), NULL);
    g_signal_connect(btn_back,  "clicked", G_CALLBACK(on_leave_pred),  NULL);

    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p, 
            "button { background: #E67E22; color: white; font-weight: bold; border-radius: 4px; }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(btn_start),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }

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
static int       s_surv_lives   = 3;
static guint     s_surv_tick    = 0;
static GtkWidget *s_surv_lbl    = NULL;
static ChampSelect *s_choice_surv_sel = NULL;

static void update_surv_hud_label(void) {
    if (!s_surv_lbl || !s_surv_bassin || !s_surv_bassin->player) return;

    /* Compter l'école de poissons (followers actifs) */
    int followers = 0;
    for (int i = 0; i < s_surv_bassin->nb_entities; i++) {
        Entity *e = s_surv_bassin->entities[i];
        if (e && e->active && e->leader == s_surv_bassin->player) {
            followers++;
        }
    }

    char buf[160];
    snprintf(buf, sizeof(buf), "⏱️ Temps : %ds  |  💖 Vies : %d/3  |  🐠 École : %d poissons  |  🌊 Vague : %d", 
             s_surv_time, s_surv_lives, followers, s_surv_wave);
    gtk_label_set_text(GTK_LABEL(s_surv_lbl), buf);
}

static void on_surv_entity_eaten(Entity *e, int score, gpointer data) {
    (void)score; (void)data;
    if (!s_surv_bassin || !s_surv_bassin->player) return;

    if (e == s_surv_bassin->player) {
        s_surv_lives--;
        update_surv_hud_label();

        if (s_surv_lives <= 0) {
            if (s_surv_tick) { g_source_remove(s_surv_tick); s_surv_tick = 0; }
            s_surv_bassin->player->active = FALSE;
            clean_keyboard_listener();
            sound_play(SOUND_GAME_OVER);
            gtk_label_set_text(GTK_LABEL(s_surv_lbl), "💀 GAME OVER ! Dévoré par les requins !");
        } else {
            // Respawn temporaire sain et sauf
            sound_play(SOUND_SHARK_ALERT);
            s_surv_bassin->player->x = s_surv_bassin->width / 2.0f;
            s_surv_bassin->player->y = s_surv_bassin->height / 2.0f;
            s_surv_bassin->player->vx = 0;
            s_surv_bassin->player->vy = 0;
        }
    } else {
        // Un poisson neutre ou suiveur de l'école est mort
        update_surv_hud_label();
    }
}

static gboolean surv_game_tick(gpointer data) {
    (void)data;
    s_surv_time++;

    /* Vagues de prédateurs système */
    if (s_surv_time % 12 == 0 && s_surv_bassin) {
        s_surv_wave = s_surv_time / 12 + 1;
        int w = s_surv_bassin->width, h = s_surv_bassin->height;
        
        /* Spawne un prédateur agressif */
        EntityType predator_types[] = {SHARK_WHITE, SHARK_MAKO, SHARK_HAMMER, ORCA};
        Entity *shark = entity_create(predator_types[rand() % 4], (float)(rand() % w), 0.0f);
        if (shark) {
            shark->aggressive = TRUE;
            shark->state = STATE_CHASING;
            shark->vx = (w / 2.0f - shark->x) / 150.0f;
            shark->vy = h / 150.0f;
            shark->angle = atan2(shark->vy, shark->vx);
            bassin_add_entity(s_surv_bassin, shark);
            sound_play(SOUND_SHARK_ALERT);
        }
    }

    /* Spawn de petits poissons sauvages périodiques pour grossir l'école */
    if (s_surv_time % 6 == 0 && s_surv_bassin && s_surv_bassin->nb_entities < 25) {
        EntityType types[] = {FISH_NORMAL, FISH_BLUE, FISH_YELLOW, FISH_MANDARIN};
        Entity *e = entity_create(types[rand() % 4], 0.0f, (float)(rand() % s_surv_bassin->height));
        if (e) {
            e->state = STATE_SWIMMING;
            e->fearful = TRUE;
            e->vx = 1.0f; e->vy = 0.2f;
            e->angle = atan2(e->vy, e->vx);
            bassin_add_entity(s_surv_bassin, e);
        }
    }

    // Le poisson recrute des followers s'il passe à côté (géré dans bassin.c update_ai)
    // On met à jour le HUD
    update_surv_hud_label();
    return G_SOURCE_CONTINUE;
}

static void surv_initialize_game_with_type(EntityType prey_type) {
    s_surv_time = 0;
    s_surv_wave = 1;
    s_surv_lives = 3;
    bassin_clear(s_surv_bassin);

    /* Créer le joueur proies */
    Entity *player = entity_create(prey_type, s_surv_bassin->width / 2.0f, s_surv_bassin->height / 2.0f);
    if (player) {
        player->is_leader = TRUE;
        player->state = STATE_IDLE;
        player->color = COLOR_ORANGE;
        bassin_add_entity(s_surv_bassin, player);
        s_surv_bassin->player = player;
        s_active_player = player;
    }

    s_surv_bassin->on_entity_eaten = on_surv_entity_eaten;
    s_surv_bassin->callback_data = NULL;

    /* Spawne 12 poissons neutres à recruter */
    for (int i = 0; i < 12; i++) {
        EntityType types[] = {FISH_NORMAL, FISH_BLUE, FISH_YELLOW, FISH_BALLOON, FISH_MANDARIN};
        Entity *e = entity_create(types[rand() % 5], (float)(rand() % (s_surv_bassin->width - 100) + 50),
                                                     (float)(rand() % (s_surv_bassin->height - 150) + 50));
        if (e) {
            e->state = STATE_SWIMMING;
            e->fearful = TRUE;
            e->vx = 1.0f * (rand() % 2 == 0 ? 1 : -1);
            e->vy = 0.5f * (rand() % 2 == 0 ? 1 : -1);
            e->angle = atan2(e->vy, e->vx);
            bassin_add_entity(s_surv_bassin, e);
        }
    }

    /* Spawne le premier prédateur système */
    Entity *shark = entity_create(SHARK_WHITE, 100.0f, 50.0f);
    if (shark) {
        shark->aggressive = TRUE;
        shark->state = STATE_CHASING;
        shark->vx = 1.0f; shark->vy = 0.5f;
        shark->angle = atan2(shark->vy, shark->vx);
        bassin_add_entity(s_surv_bassin, shark);
    }

    setup_keyboard_listener();
    sound_play(SOUND_SPLASH);

    if (s_surv_tick) g_source_remove(s_surv_tick);
    s_surv_tick = g_timeout_add(1000, surv_game_tick, NULL);
    update_surv_hud_label();
}

static void on_choice_surv_reponse(int reponse_id, gpointer user_data) {
    Dialog *dlg = (Dialog *)user_data;
    EntityType chosen_type = FISH_CLOWN;

    if (reponse_id == DIALOG_REPONSE_OUI || reponse_id == DIALOG_REPONSE_OK) {
        int idx = s_choice_surv_sel->selected_index;
        switch (idx) {
            case 0: chosen_type = FISH_CLOWN; break;
            case 1: chosen_type = FISH_BALLOON; break;
            case 2: chosen_type = FISH_BLUE; break;
            case 3: chosen_type = FISH_YELLOW; break;
            case 4: chosen_type = FISH_MANDARIN; break;
        }
    }

    g_free(s_choice_surv_sel);
    s_choice_surv_sel = NULL;
    surv_initialize_game_with_type(chosen_type);
}

static void surv_start_game(GtkButton *btn, gpointer data) {
    (void)btn; (void)data;

    Dialog *dlg = g_new0(Dialog, 1);
    dialog_initialiser(dlg);
    dlg->parent = GTK_WINDOW(g_window);
    dlg->type = DIALOG_INFO;
    dlg->titre = g_strdup("🐟 Mode Survie — Choisissez votre Poisson");
    dlg->message = g_strdup("Quel poisson souhaitez-vous incarner pour échapper aux requins ?");
    dlg->boutons_preset = DIALOG_BOUTONS_OUI_NON;
    dlg->on_reponse = on_choice_surv_reponse;
    dlg->user_data = dlg;

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(content, 15);
    gtk_widget_set_margin_end(content, 15);
    
    s_choice_surv_sel = g_new0(ChampSelect, 1);
    champ_select_initialiser(s_choice_surv_sel);
    s_choice_surv_sel->selected_index = 0;
    champ_select_add_item(s_choice_surv_sel, "🐠 Poisson Clown (Nemo)");
    champ_select_add_item(s_choice_surv_sel, "🐡 Poisson Ballon");
    champ_select_add_item(s_choice_surv_sel, "🐟 Poisson Bleu");
    champ_select_add_item(s_choice_surv_sel, "💛 Poisson Jaune");
    champ_select_add_item(s_choice_surv_sel, "🎨 Poisson Mandarin");

    GtkWidget *w_sel = champ_select_creer(s_choice_surv_sel);
    gtk_box_append(GTK_BOX(content), w_sel);
    dlg->widget_contenu = content;

    dialog_creer(dlg);
    dialog_afficher(dlg);
    g_signal_connect_swapped(dlg->window, "destroy", G_CALLBACK(dialog_free), dlg);
}

static void on_leave_surv(GtkButton *btn, gpointer data) {
    (void)btn; (void)data;
    clean_keyboard_listener();
    if (s_surv_tick) { g_source_remove(s_surv_tick); s_surv_tick = 0; }
    nav_to_accueil();
}

GtkWidget *screen_survie_create(void) {
    s_surv_bassin = bassin_create(BASSIN_MODE_SURVIE);
    bassin_start(s_surv_bassin);

    GtkWidget *overlay = gtk_overlay_new();
    GtkWidget *canvas = bassin_get_widget(s_surv_bassin);
    gtk_overlay_set_child(GTK_OVERLAY(overlay), canvas);

    /* Gestionnaire souris */
    GtkEventController *motion = gtk_event_controller_motion_new();
    g_signal_connect(motion, "motion", G_CALLBACK(on_bassin_motion), (gpointer)s_surv_bassin);
    gtk_widget_add_controller(canvas, motion);

    /* HUD */
    GtkWidget *hud = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(hud, GTK_ALIGN_FILL);
    gtk_widget_set_valign(hud, GTK_ALIGN_START);

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p,
            "box { background: rgba(5,15,30,0.85); padding: 12px; border-bottom: 2px solid rgba(255,255,255,0.08); }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(top_bar),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }

    s_surv_lbl = gtk_label_new("Sélectionnez votre poisson pour lancer le défi de survie !");
    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p,
            "label { color: #FFFFFF; font-size: 18px; font-weight: bold; }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(s_surv_lbl),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }

    GtkWidget *btn_start = gtk_button_new_with_label("▶ Choisir & Démarrer");
    GtkWidget *btn_back  = gtk_button_new_with_label("← Accueil Menu");
    g_signal_connect(btn_start, "clicked", G_CALLBACK(surv_start_game), NULL);
    g_signal_connect(btn_back,  "clicked", G_CALLBACK(on_leave_surv),  NULL);

    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p, 
            "button { background: #27AE60; color: white; font-weight: bold; border-radius: 4px; }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(btn_start),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }

    gtk_box_append(GTK_BOX(top_bar), btn_start);
    gtk_box_append(GTK_BOX(top_bar), btn_back);
    gtk_box_append(GTK_BOX(top_bar), s_surv_lbl);
    gtk_box_append(GTK_BOX(hud), top_bar);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), hud);
    return overlay;
}