#include "screen_createur.h"
#include "bassin.h"
#include "sound.h"
#include "entities.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../widgets/headers/common.h"
#include "../widgets/headers/champ_select.h"
#include "../widgets/headers/bouton_checklist.h"
#include "../widgets/headers/dialog.h"

extern void nav_to_accueil(void);
extern GtkWidget *g_window;
extern gboolean g_creator_paused;

static Bassin    *s_bassin    = NULL;
static EntityType s_tool_type = FISH_NORMAL; /* -1 correspond au mode Gomme/Eraser */
static GtkWidget *s_label_count = NULL;

/* Formulaire Dialog Pointers */
static ChampSelect *s_form_type_sel = NULL;
static ChampSelect *s_form_color_sel = NULL;
static ChampSelect *s_form_size_sel = NULL;
static BoutonChecklist *s_form_fearful_chk = NULL;
static BoutonChecklist *s_form_aggressive_chk = NULL;
static BoutonChecklist *s_form_leader_chk = NULL;

static void update_count_label(void) {
    if (!s_label_count || !s_bassin) return;
    int fish   = 0, sharks = 0, objects = 0;
    for (int i = 0; i < s_bassin->nb_entities; i++) {
        Entity *e = s_bassin->entities[i];
        if (!e || !e->active) continue;
        if (entity_is_predator(e->type))      sharks++;
        else if (entity_is_object(e->type))   objects++;
        else                                  fish++;
    }
    char buf[128];
    snprintf(buf, sizeof(buf), "🐠 %d | 🦈 %d | 🪨 %d", fish, sharks, objects);
    gtk_label_set_text(GTK_LABEL(s_label_count), buf);
}

static void on_bassin_click(GtkGestureClick *gc, int n, double x, double y, gpointer data) {
    (void)gc; (void)n; (void)data;
    if (!s_bassin) return;

    if ((int)s_tool_type == -1) {
        /* 🗑️ Mode Gomme/Eraser : Supprimer l'élément le plus proche sous le clic */
        Entity *nearest = NULL;
        double min_dist = 99999.0;
        for (int i = 0; i < s_bassin->nb_entities; i++) {
            Entity *e = s_bassin->entities[i];
            if (!e || !e->active) continue;
            double dx = e->x - x;
            double dy = e->y - y;
            double dist = sqrt(dx*dx + dy*dy);
            if (dist < min_dist) {
                min_dist = dist;
                nearest = e;
            }
        }
        if (nearest && min_dist < 40.0) {
            bassin_remove_entity(s_bassin, nearest);
            entity_free(nearest);
            sound_play(SOUND_SPLASH);
            update_count_label();
            gtk_widget_queue_draw(bassin_get_widget(s_bassin));
        }
    } else {
        /* Mode Placement rapide */
        Entity *e = entity_create(s_tool_type, (float)x, (float)y);
        if (e) {
            e->state = STATE_SWIMMING;
            
            // Configs intelligentes par défaut
            if (entity_is_predator(s_tool_type)) {
                e->aggressive = TRUE;
                e->color = COLOR_RED;
            } else if (!entity_is_object(s_tool_type)) {
                e->fearful = TRUE;
                e->color = (EntityColor)(rand() % 8);
            }
            
            e->vx = 1.5f * (rand() % 2 == 0 ? 1 : -1);
            e->vy = 0.5f * (rand() % 2 == 0 ? 1 : -1);
            e->angle = atan2(e->vy, e->vx);
            bassin_add_entity(s_bassin, e);
            update_count_label();
            gtk_widget_queue_draw(bassin_get_widget(s_bassin));
        }
    }
}

static void set_tool(GtkButton *btn, gpointer data) {
    (void)btn;
    s_tool_type = (EntityType)(intptr_t)data;
    printf("[OUTIL] Outil actif : %s\n", 
           (int)s_tool_type == -1 ? "Gomme/Supprimer" : entity_get_config(s_tool_type)->name_fr);
}

static void on_reset(GtkButton *btn, gpointer data) {
    (void)btn; (void)data;
    bassin_clear(s_bassin);
    update_count_label();
    gtk_widget_queue_draw(bassin_get_widget(s_bassin));
}

static void on_save(GtkButton *btn, gpointer data) {
    (void)btn; (void)data;
    bassin_save(s_bassin, "data/bassin.txt");
    printf("[CRÉATEUR] Bassin sauvegardé sous data/bassin.txt\n");
}

static void on_load(GtkButton *btn, gpointer data) {
    (void)btn; (void)data;
    bassin_load(s_bassin, "data/bassin.txt");
    update_count_label();
    gtk_widget_queue_draw(bassin_get_widget(s_bassin));
    printf("[CRÉATEUR] Bassin chargé depuis data/bassin.txt\n");
}

static void on_toggle_pause(GtkButton *btn, gpointer data) {
    (void)data;
    g_creator_paused = !g_creator_paused;
    if (g_creator_paused) {
        gtk_button_set_label(btn, "▶️ Lecture");
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p, 
            "button { background: #27AE60; color: white; border-radius: 6px; font-weight: bold; }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(GTK_WIDGET(btn)),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    } else {
        gtk_button_set_label(btn, "⏸️ Pause");
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p, 
            "button { background: #E67E22; color: white; border-radius: 6px; font-weight: bold; }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(GTK_WIDGET(btn)),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }
}

/* Callback de validation du formulaire d'ajout */
static void on_dialog_ajouter_reponse(int reponse_id, gpointer user_data) {
    Dialog *dlg = (Dialog *)user_data;
    if (reponse_id == DIALOG_REPONSE_OUI || reponse_id == DIALOG_REPONSE_OK) {
        if (!s_bassin || !s_form_type_sel || !s_form_color_sel || !s_form_size_sel) return;

        int type_idx = s_form_type_sel->selected_index;
        int color_idx = s_form_color_sel->selected_index;
        int size_idx = s_form_size_sel->selected_index;

        if (type_idx < 0 || type_idx >= ENTITY_COUNT) type_idx = 0;
        if (color_idx < 0 || color_idx > 7) color_idx = 0;
        if (size_idx < 0 || size_idx > 4) size_idx = 2;

        EntityType type = (EntityType)type_idx;
        EntityColor color = (EntityColor)color_idx;
        EntitySize size = (EntitySize)size_idx;

        gboolean fearful = (s_form_fearful_chk->etat == CHECKLIST_CHECKED);
        gboolean aggressive = (s_form_aggressive_chk->etat == CHECKLIST_CHECKED);
        gboolean is_leader = (s_form_leader_chk->etat == CHECKLIST_CHECKED);

        /* Placer au centre du bassin avec un petit décalage aléatoire */
        float cx = s_bassin->width / 2.0f + (rand() % 120 - 60);
        float cy = s_bassin->height / 2.0f + (rand() % 120 - 60);

        Entity *e = entity_create(type, cx, cy);
        if (e) {
            e->color = color;
            e->size = size;
            
            switch (size) {
                case SIZE_TINY:   e->size_mult = 0.5f; break;
                case SIZE_SMALL:  e->size_mult = 0.75f; break;
                case SIZE_MEDIUM: e->size_mult = 1.0f; break;
                case SIZE_LARGE:  e->size_mult = 1.5f; break;
                case SIZE_HUGE:   e->size_mult = 2.2f; break;
            }

            e->fearful = fearful;
            e->aggressive = aggressive;
            e->is_leader = is_leader;
            
            e->state = STATE_SWIMMING;
            e->vx = 1.5f * (rand() % 2 == 0 ? 1 : -1);
            e->vy = 0.5f * (rand() % 2 == 0 ? 1 : -1);
            e->angle = atan2(e->vy, e->vx);

            bassin_add_entity(s_bassin, e);
            update_count_label();
            gtk_widget_queue_draw(bassin_get_widget(s_bassin));
            printf("[CRÉATEUR] Entité configurée ajoutée : %s\n", entity_get_config(type)->name_fr);
        }
    }

    // Libérer la mémoire des structures temporaires du formulaire
    g_free(s_form_fearful_chk->label);
    g_free(s_form_fearful_chk);
    g_free(s_form_aggressive_chk->label);
    g_free(s_form_aggressive_chk);
    g_free(s_form_leader_chk->label);
    g_free(s_form_leader_chk);
    g_free(s_form_type_sel);
    g_free(s_form_color_sel);
    g_free(s_form_size_sel);

    s_form_fearful_chk = NULL;
    s_form_aggressive_chk = NULL;
    s_form_leader_chk = NULL;
    s_form_type_sel = NULL;
    s_form_color_sel = NULL;
    s_form_size_sel = NULL;
}

static void on_ajouter_formulaire_click(GtkButton *btn, gpointer data) {
    (void)btn; (void)data;
    
    Dialog *dlg = g_new0(Dialog, 1);
    dialog_initialiser(dlg);
    dlg->parent = GTK_WINDOW(g_window);
    dlg->type = DIALOG_INFO;
    dlg->titre = g_strdup("➕ Configurer & Ajouter une Entité");
    dlg->message = g_strdup("Remplissez le formulaire d'ajout :");
    dlg->boutons_preset = DIALOG_BOUTONS_OUI_NON;
    dlg->on_reponse = on_dialog_ajouter_reponse;
    dlg->user_data = dlg;

    GtkWidget *form_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(form_box, 15);
    gtk_widget_set_margin_end(form_box, 15);
    gtk_widget_set_margin_top(form_box, 10);
    gtk_widget_set_margin_bottom(form_box, 10);

    /* 1. Type dropdown */
    GtkWidget *row_type = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *lbl_type = gtk_label_new("Type :");
    gtk_widget_set_size_request(lbl_type, 90, -1);
    gtk_box_append(GTK_BOX(row_type), lbl_type);
    
    s_form_type_sel = g_new0(ChampSelect, 1);
    champ_select_initialiser(s_form_type_sel);
    s_form_type_sel->selected_index = 0;
    for (int t = 0; t < ENTITY_COUNT; t++) {
        champ_select_add_item(s_form_type_sel, entity_get_config(t)->name_fr);
    }
    GtkWidget *w_type = champ_select_creer(s_form_type_sel);
    gtk_widget_set_hexpand(w_type, TRUE);
    gtk_box_append(GTK_BOX(row_type), w_type);
    gtk_box_append(GTK_BOX(form_box), row_type);

    /* 2. Couleur */
    GtkWidget *row_color = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *lbl_color = gtk_label_new("Couleur :");
    gtk_widget_set_size_request(lbl_color, 90, -1);
    gtk_box_append(GTK_BOX(row_color), lbl_color);
    
    s_form_color_sel = g_new0(ChampSelect, 1);
    champ_select_initialiser(s_form_color_sel);
    s_form_color_sel->selected_index = 1; /* Bleu */
    champ_select_add_item(s_form_color_sel, "Rouge");
    champ_select_add_item(s_form_color_sel, "Bleu");
    champ_select_add_item(s_form_color_sel, "Jaune");
    champ_select_add_item(s_form_color_sel, "Vert");
    champ_select_add_item(s_form_color_sel, "Orange");
    champ_select_add_item(s_form_color_sel, "Blanc");
    champ_select_add_item(s_form_color_sel, "Violet");
    champ_select_add_item(s_form_color_sel, "Cyan");
    GtkWidget *w_color = champ_select_creer(s_form_color_sel);
    gtk_widget_set_hexpand(w_color, TRUE);
    gtk_box_append(GTK_BOX(row_color), w_color);
    gtk_box_append(GTK_BOX(form_box), row_color);

    /* 3. Taille */
    GtkWidget *row_size = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *lbl_size = gtk_label_new("Taille :");
    gtk_widget_set_size_request(lbl_size, 90, -1);
    gtk_box_append(GTK_BOX(row_size), lbl_size);
    
    s_form_size_sel = g_new0(ChampSelect, 1);
    champ_select_initialiser(s_form_size_sel);
    s_form_size_sel->selected_index = 2; /* Medium */
    champ_select_add_item(s_form_size_sel, "Minuscule");
    champ_select_add_item(s_form_size_sel, "Petit");
    champ_select_add_item(s_form_size_sel, "Moyen");
    champ_select_add_item(s_form_size_sel, "Grand");
    champ_select_add_item(s_form_size_sel, "Géant");
    GtkWidget *w_size = champ_select_creer(s_form_size_sel);
    gtk_widget_set_hexpand(w_size, TRUE);
    gtk_box_append(GTK_BOX(row_size), w_size);
    gtk_box_append(GTK_BOX(form_box), row_size);

    /* 4. Comportements */
    GtkWidget *lbl_beh = gtk_label_new("<b>Comportements & IA :</b>");
    gtk_label_set_use_markup(GTK_LABEL(lbl_beh), TRUE);
    gtk_widget_set_margin_top(lbl_beh, 10);
    gtk_box_append(GTK_BOX(form_box), lbl_beh);

    s_form_fearful_chk = g_new0(BoutonChecklist, 1);
    bouton_checklist_initialiser(s_form_fearful_chk);
    s_form_fearful_chk->label = g_strdup("Fuit les prédateurs (espace de sécurité)");
    s_form_fearful_chk->etat = CHECKLIST_CHECKED;
    GtkWidget *w_fear = bouton_checklist_creer(s_form_fearful_chk);
    gtk_box_append(GTK_BOX(form_box), w_fear);

    s_form_aggressive_chk = g_new0(BoutonChecklist, 1);
    bouton_checklist_initialiser(s_form_aggressive_chk);
    s_form_aggressive_chk->label = g_strdup("Agressif (chasse activement les proies)");
    s_form_aggressive_chk->etat = CHECKLIST_UNCHECKED;
    GtkWidget *w_agg = bouton_checklist_creer(s_form_aggressive_chk);
    gtk_box_append(GTK_BOX(form_box), w_agg);

    s_form_leader_chk = g_new0(BoutonChecklist, 1);
    bouton_checklist_initialiser(s_form_leader_chk);
    s_form_leader_chk->label = g_strdup("Chef de banc (d'autres poissons le suivront)");
    s_form_leader_chk->etat = CHECKLIST_UNCHECKED;
    GtkWidget *w_lead = bouton_checklist_creer(s_form_leader_chk);
    gtk_box_append(GTK_BOX(form_box), w_lead);

    dlg->widget_contenu = form_box;
    dialog_creer(dlg);
    dialog_afficher(dlg);
    g_signal_connect_swapped(dlg->window, "destroy", G_CALLBACK(dialog_free), dlg);
}

static GtkWidget *make_tool_btn(const char *label, EntityType type, const char *color) {
    GtkWidget *btn = gtk_button_new_with_label(label);
    gtk_widget_set_size_request(btn, 140, 36);
    
    char css[256];
    snprintf(css, sizeof(css), 
             "button { background: %s; color: white; border-radius: 4px; font-weight: bold; border: none; }"
             "button:hover { background: shade(%s, 1.15); }",
             color, color);
    
    GtkCssProvider *p = gtk_css_provider_new();
    gtk_css_provider_load_from_string(p, css);
    gtk_style_context_add_provider(gtk_widget_get_style_context(btn),
        GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(p);

    g_signal_connect(btn, "clicked", G_CALLBACK(set_tool), (gpointer)(intptr_t)type);
    return btn;
}

GtkWidget *screen_createur_create(void) {
    s_bassin = bassin_create(BASSIN_MODE_CREATEUR);
    bassin_start(s_bassin);
    g_creator_paused = FALSE;

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    /* ── Barre gauche (outils) ── */
    GtkWidget *left = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_size_request(left, 180, -1);
    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p,
            "box { background: rgba(10,25,47,0.92); padding: 12px; border-right: 2px solid rgba(255,255,255,0.08); }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(left),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }

    GtkWidget *lbl_title = gtk_label_new("🌊 CRÉATEUR");
    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p, "label { color: #3498DB; font-size: 20px; font-weight: bold; }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(lbl_title),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }
    gtk_box_append(GTK_BOX(left), lbl_title);

    /* Bouton principal Ajouter Formulaire */
    GtkWidget *btn_form = gtk_button_new_with_label("➕ Ajouter Spec.");
    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p, 
            "button { background: #8E44AD; color: white; font-weight: bold; border-radius: 6px; font-size: 14px; }"
            "button:hover { background: #9B59B6; }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(btn_form),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }
    g_signal_connect(btn_form, "clicked", G_CALLBACK(on_ajouter_formulaire_click), NULL);
    gtk_box_append(GTK_BOX(left), btn_form);

    GtkWidget *sep_tool = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(left), sep_tool);

    /* Outils d'ajout rapide */
    gtk_box_append(GTK_BOX(left), make_tool_btn("🐟 Poisson Bleu", FISH_BLUE,    "#3498DB"));
    gtk_box_append(GTK_BOX(left), make_tool_btn("🐠 Poisson Clown", FISH_CLOWN,  "#E67E22"));
    gtk_box_append(GTK_BOX(left), make_tool_btn("🐡 Ballon",       FISH_BALLOON, "#F1C40F"));
    gtk_box_append(GTK_BOX(left), make_tool_btn("🦈 Grand Requin", SHARK_WHITE, "#7F8C8D"));
    gtk_box_append(GTK_BOX(left), make_tool_btn("🐢 Tortue de Mer", TURTLE_SEA,  "#27AE60"));
    gtk_box_append(GTK_BOX(left), make_tool_btn("🎐 Méduse",       JELLYFISH,   "#9B59B6"));
    gtk_box_append(GTK_BOX(left), make_tool_btn("🪨 Rocher",       OBJ_ROCK,    "#5D6D7E"));
    gtk_box_append(GTK_BOX(left), make_tool_btn("🌿 Algue Verte",   OBJ_ALGAE,   "#1E824C"));
    gtk_box_append(GTK_BOX(left), make_tool_btn("🪸 Corail Rose",   OBJ_CORAL,   "#EF4836"));

    /* Gomme */
    gtk_box_append(GTK_BOX(left), make_tool_btn("🗑️ Gomme / Del", (EntityType)-1, "#C0392B"));

    GtkWidget *sep_action = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(left), sep_action);

    /* Lecture/Pause Simulation */
    GtkWidget *btn_pause = gtk_button_new_with_label("⏸️ Pause");
    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p, 
            "button { background: #E67E22; color: white; border-radius: 6px; font-weight: bold; }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(btn_pause),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }
    g_signal_connect(btn_pause, "clicked", G_CALLBACK(on_toggle_pause), NULL);
    gtk_box_append(GTK_BOX(left), btn_pause);

    /* Vider */
    GtkWidget *btn_reset = gtk_button_new_with_label("🔄 Reset");
    g_signal_connect(btn_reset, "clicked", G_CALLBACK(on_reset), NULL);
    gtk_box_append(GTK_BOX(left), btn_reset);

    /* Retour menu */
    GtkWidget *btn_back = gtk_button_new_with_label("← Accueil Menu");
    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p, 
            "button { background: #7F8C8D; color: white; font-weight: bold; }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(btn_back),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }
    g_signal_connect(btn_back, "clicked", G_CALLBACK(nav_to_accueil), NULL);
    gtk_box_append(GTK_BOX(left), btn_back);

    /* ── Zone centrale (bassin) ── */
    GtkWidget *center = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(center, TRUE);
    gtk_widget_set_vexpand(center, TRUE);

    GtkWidget *canvas = bassin_get_widget(s_bassin);
    gtk_widget_set_hexpand(canvas, TRUE);
    gtk_widget_set_vexpand(canvas, TRUE);

    GtkGesture *click = gtk_gesture_click_new();
    g_signal_connect(click, "pressed", G_CALLBACK(on_bassin_click), NULL);
    gtk_widget_add_controller(canvas, GTK_EVENT_CONTROLLER(click));

    gtk_box_append(GTK_BOX(center), canvas);

    /* Barre du bas */
    GtkWidget *bottom = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p,
            "box { background: rgba(5,15,30,0.95); padding: 12px; border-top: 1px solid rgba(255,255,255,0.08); }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(bottom),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }

    GtkWidget *btn_save = gtk_button_new_with_label("💾 Sauvegarder");
    GtkWidget *btn_load = gtk_button_new_with_label("📂 Charger");
    g_signal_connect(btn_save, "clicked", G_CALLBACK(on_save), NULL);
    g_signal_connect(btn_load, "clicked", G_CALLBACK(on_load), NULL);

    s_label_count = gtk_label_new("🐠 0 | 🦈 0 | 🪨 0");
    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p, "label { color: #2ECC71; font-weight: bold; font-size: 16px; }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(s_label_count),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }

    gtk_box_append(GTK_BOX(bottom), btn_save);
    gtk_box_append(GTK_BOX(bottom), btn_load);
    gtk_box_append(GTK_BOX(bottom), s_label_count);

    gtk_box_append(GTK_BOX(center), bottom);

    /* Assemblage final */
    gtk_box_append(GTK_BOX(hbox), left);
    gtk_box_append(GTK_BOX(hbox), center);

    update_count_label();

    return hbox;
}