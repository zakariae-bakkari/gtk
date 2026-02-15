#include "../headers/fenetre.h"
#include <stdio.h>
#include <string.h>
#include <gdk-pixbuf/gdk-pixbuf.h> // pour charger des images
/* Fonction interne pour le CSS */
static void _fenetre_appliquer_css(GtkWidget *window, Fenetre *config) {
    GtkCssProvider *provider = gtk_css_provider_new();
    char css_data[1024];

    if (config->background_image != NULL) {
        snprintf(css_data, sizeof(css_data),
            "window { background-image: url('%s'); background-size: cover; background-position: center; }",
            config->background_image);
    } else if (config->color_bg != NULL) {
        snprintf(css_data, sizeof(css_data),
            "window { background-color: %s; }",
            config->color_bg);
    } else {
        g_object_unref(provider);
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
    config->titre_align = TITRE_ALIGN_CENTRE;
    config->icon_path = NULL;
    config->type = WIN_TYPE_TOPLEVEL;
    config->resizable = true;
    config->demarrer_maximisee = false;
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

    config->wind = gtk_window_new();// GTK4 : gtk_window_new() crée une fenêtre toplevel par défaut
    gtk_window_set_default_size(GTK_WINDOW(config->wind), config->taille.width, config->taille.height);// Taille par défaut
    gtk_window_set_resizable(GTK_WINDOW(config->wind), config->resizable);// Redimensionnable ou pas

    if (config->demarrer_maximisee) {
        gtk_window_maximize(GTK_WINDOW(config->wind));// Démarrer maximisée
    }

    // --- CRÉATION DE LA BARRE D'EN-TÊTE ---
    GtkWidget *header_bar = gtk_header_bar_new();// Crée une nouvelle HeaderBar
    gtk_header_bar_set_show_title_buttons(GTK_HEADER_BAR(header_bar), TRUE);// Affiche les boutons de contrôle (minimiser, maximiser, fermer) par défaut

    // --- CORRECTION DU LAYOUT DES BOUTONS ---
    // On initialise avec ":" pour dire "RIEN À GAUCHE"
    char layout_desc[100];
    strcpy(layout_desc, ":");

    if (config->bouton_reduire) strcat(layout_desc, "minimize,");// Ajouter le bouton de réduction
    if (config->bouton_agrandir && config->resizable) strcat(layout_desc, "maximize,");// Ajouter le bouton d'agrandissement (uniquement si la fenêtre est redimensionnable)
    if (config->bouton_fermer) strcat(layout_desc, "close");// Ajouter le bouton de fermeture

    // Supprimer la virgule finale si elle existe
    size_t len = strlen(layout_desc);
    if (len > 1 && layout_desc[len-1] == ',') {
        layout_desc[len-1] = '\0';
    }

    // DEBUG : Affiche ce que le programme essaie de faire
    printf("\n[DEBUG] Configuration des boutons : '%s'\n", layout_desc);

    gtk_header_bar_set_decoration_layout(GTK_HEADER_BAR(header_bar), layout_desc);

    // --- TITRE ---
    GtkWidget *custom_title = gtk_label_new(config->title);
    float xalign = 0.5;
    if (config->titre_align == TITRE_ALIGN_GAUCHE) xalign = 0.0;
    if (config->titre_align == TITRE_ALIGN_DROITE) xalign = 1.0;
    gtk_label_set_xalign(GTK_LABEL(custom_title), xalign);
    gtk_widget_set_hexpand(custom_title, TRUE);
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(header_bar), custom_title);

    // Attacher la barre à la fenêtre
    gtk_window_set_titlebar(GTK_WINDOW(config->wind), header_bar);

    if (config->icon_path) {
        // Vérifier si c'est un chemin de fichier (contient un '/' ou un '.')
        if (g_file_test(config->icon_path, G_FILE_TEST_EXISTS)) {
            GError *error = NULL;
            // Charger le fichier en texture
            GdkTexture *texture = gdk_texture_new_from_filename(config->icon_path, &error);

            if (texture) {
                // Note: En GTK4, on ne peut plus définir l'icône d'une SEULE fenêtre via un fichier facilement
                // car l'API a changé. La méthode standard est d'utiliser le thème d'icônes.
                // Mais pour un fichier spécifique, on utilise souvent un logo dans la HeaderBar.
                g_object_unref(texture);
            } else {
                g_warning("Erreur chargement icone : %s", error->message);
                g_error_free(error);
            }
        } else {
            // C'est un nom d'icône système (ex: "computer")
            gtk_window_set_icon_name(GTK_WINDOW(config->wind), config->icon_path);
        }

    }

    _fenetre_appliquer_css(config->wind, config);

    return config->wind;
}
