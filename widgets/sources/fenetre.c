#include "../headers/fenetre.h"
#include <stdio.h>
#include <string.h>

/* Fonction interne pour le CSS (inchangée) */
static void _fenetre_appliquer_css(GtkWidget *window, Fenetre *config) {
    GtkCssProvider *provider = gtk_css_provider_new();
    char css_data[1024];

    if (config->background_image != NULL) {
        snprintf(css_data, sizeof(css_data),
            "window { background-image: url('%s'); background-size: cover; }",
            config->background_image);
    } else if (config->color_bg != NULL) {
        snprintf(css_data, sizeof(css_data),
            "window { background-color: %s; }",
            config->color_bg);
    } else {
        return;
    }

    gtk_css_provider_load_from_string(provider, css_data);
    GtkStyleContext *context = gtk_widget_get_style_context(window);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);
}

void fenetre_initialiser(Fenetre *config) {
    if (!config) return;

    config->wind = NULL;
    config->title = "Application";
    config->titre_align = TITRE_ALIGN_CENTRE; // Par défaut au centre
    config->icon_path = NULL;
    config->type = WIN_TYPE_TOPLEVEL;

    config->resizable = true;
    config->demarrer_maximisee = false; // Par défaut taille normale

    // Par défaut, on affiche tout
    config->bouton_fermer = true;
    config->bouton_agrandir = true;
    config->bouton_reduire = true;

    config->taille.width = 800;
    config->taille.height = 600;
    config->color_bg = NULL;
    config->background_image = NULL;
    config->position = WIN_POS_CENTER;
    config->id = 0;
}

GtkWidget* fenetre_creer(Fenetre *config) {
    if (!config) return NULL;

    // 1. Création de la fenêtre
    config->wind = gtk_window_new();

    // 2. Taille et Redimensionnement
    gtk_window_set_default_size(GTK_WINDOW(config->wind), config->taille.width, config->taille.height);
    gtk_window_set_resizable(GTK_WINDOW(config->wind), config->resizable);

    // 3. Gestion Maximisée (Prendre toute la taille)
    if (config->demarrer_maximisee) {
        gtk_window_maximize(GTK_WINDOW(config->wind));
    }

    // 4. Création d'une HeaderBar personnalisée pour gérer Titre et Boutons
    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_header_bar_set_show_title_buttons(GTK_HEADER_BAR(header_bar), TRUE);

    // --- Gestion des Boutons (Symboles) ---
    // On construit une chaîne de layout. Ex: "minimize,maximize,close" ou "close"
    char layout_desc[64] = "";

    // On ajoute les boutons à gauche ou droite selon l'OS (ici style standard : boutons à droite)
    // Note : En GTK4 strict, modifier le layout peut dépendre des settings,
    // mais voici l'approche via GtkHeaderBar.

    if (config->bouton_reduire) strcat(layout_desc, "minimize,");
    if (config->bouton_agrandir && config->resizable) strcat(layout_desc, "maximize,");
    if (config->bouton_fermer) strcat(layout_desc, "close");

    // Astuce : En GTK4, set_decoration_layout s'applique au layout manager.
    gtk_header_bar_set_decoration_layout(GTK_HEADER_BAR(header_bar), layout_desc);

    // --- Gestion de la Position du Titre ---
    // Pour aligner le titre, on crée notre propre Label au lieu d'utiliser celui par défaut
    GtkWidget *custom_title = gtk_label_new(config->title);

    // Définition de l'alignement (0.0 = Gauche, 0.5 = Centre, 1.0 = Droite)
    float xalign = 0.5; // Centre par défaut
    if (config->titre_align == TITRE_ALIGN_GAUCHE) xalign = 0.0;
    if (config->titre_align == TITRE_ALIGN_DROITE) xalign = 1.0;

    gtk_label_set_xalign(GTK_LABEL(custom_title), xalign);

    // Si on veut que l'alignement fonctionne bien, le label doit prendre de la place
    gtk_widget_set_hexpand(custom_title, TRUE);

    // On ajoute le titre à la barre
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(header_bar), custom_title);

    // On attache la HeaderBar à la fenêtre
    gtk_window_set_titlebar(GTK_WINDOW(config->wind), header_bar);

    // 5. Icône
    if (config->icon_path) {
        gtk_window_set_icon_name(GTK_WINDOW(config->wind), config->icon_path);
    }

    // 6. Style
    _fenetre_appliquer_css(config->wind, config);

    return config->wind;
}
