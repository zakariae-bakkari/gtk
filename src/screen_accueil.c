#include "screen_accueil.h"
#include "bassin.h"
#include "draw.h"
#include <math.h>

/* navigation déclarée dans main.c */
extern void nav_to_createur  (void);
extern void nav_to_predateur (void);
extern void nav_to_survie    (void);

static Bassin *s_bassin = NULL;

/* ── Boutons de mode ─────────────────────────────── */
static GtkWidget *make_mode_button(const char *label, const char *color_hex,
                                   GCallback cb) {
    GtkWidget *btn = gtk_button_new_with_label(label);
    gtk_widget_set_size_request(btn, 220, 70);

    char css[256];
    snprintf(css, sizeof(css),
        "button { background: %s; color: white; border-radius: 35px;"
        " font-size: 18px; font-weight: bold; border: 3px solid rgba(255,255,255,0.4); }"
        "button:hover { background: shade(%s, 1.2); }",
        color_hex, color_hex);

    GtkCssProvider *prov = gtk_css_provider_new();
    gtk_css_provider_load_from_string(prov, css);
    gtk_style_context_add_provider(gtk_widget_get_style_context(btn),
        GTK_STYLE_PROVIDER(prov), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(prov);

    g_signal_connect(btn, "clicked", cb, NULL);
    return btn;
}

GtkWidget *screen_accueil_create(void) {
    /* Overlay : bassin animé en fond + UI par-dessus */
    GtkWidget *overlay = gtk_overlay_new();

    s_bassin = bassin_create(BASSIN_MODE_ACCUEIL);

    /* Peupler le bassin décoratif avec un banc */
    for (int i = 0; i < 8; i++) {
        Entity *e = entity_create(FISH_NORMAL, 100 + i * 80, 250 + (i % 3) * 30);
        e->state = STATE_SWIMMING;
        e->color = (EntityColor)(i % 4);
        e->vx = 1.5f + (i % 3) * 0.3f;
        e->vy = 0.2f * (i % 2 == 0 ? 1 : -1);
        e->angle = atan2(e->vy, e->vx);
        if (i == 0) e->is_leader = TRUE;
        else        e->leader = s_bassin->entities[0];
        bassin_add_entity(s_bassin, e);
    }
    bassin_start(s_bassin);

    gtk_overlay_set_child(GTK_OVERLAY(overlay), bassin_get_widget(s_bassin));

    /* UI par-dessus */
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 30);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);

    /* Titre */
    GtkWidget *title = gtk_label_new("🐠 BANC DE POISSON");
    gtk_label_set_use_markup(GTK_LABEL(title), FALSE);
    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p,
            "label { color: white; font-size: 42px; font-weight: bold;"
            " text-shadow: 2px 2px 8px rgba(0,100,200,0.8); }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(title),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }
    gtk_box_append(GTK_BOX(vbox), title);

    GtkWidget *subtitle = gtk_label_new("Choisissez votre mode de jeu");
    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p,
            "label { color: rgba(255,255,255,0.8); font-size: 16px; }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(subtitle),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }
    gtk_box_append(GTK_BOX(vbox), subtitle);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_halign(hbox, GTK_ALIGN_CENTER);

    GtkWidget *btn_cr = make_mode_button("🎨 Mode Créateur",  "#2ECC71",
                                         G_CALLBACK(nav_to_createur));
    GtkWidget *btn_pr = make_mode_button("🦈 Mode Prédateur", "#E74C3C",
                                         G_CALLBACK(nav_to_predateur));
    GtkWidget *btn_su = make_mode_button("🐠 Mode Survie",    "#F39C12",
                                         G_CALLBACK(nav_to_survie));

    gtk_box_append(GTK_BOX(hbox), btn_cr);
    gtk_box_append(GTK_BOX(hbox), btn_pr);
    gtk_box_append(GTK_BOX(hbox), btn_su);
    gtk_box_append(GTK_BOX(vbox), hbox);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), vbox);
    return overlay;
}