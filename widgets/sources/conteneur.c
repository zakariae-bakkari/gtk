#include "../headers/conteneur.h"
#include <stdio.h>

/* Fonction interne pour appliquer la couleur de fond via CSS */
static void _conteneur_appliquer_css(GtkWidget *widget, Conteneur *config) {
    if (!config->couleur_fond) return;

    GtkCssProvider *provider = gtk_css_provider_new();
    char css_data[512];

    // "box" cible le noeud CSS de la GtkBox
    snprintf(css_data, sizeof(css_data),
        "box { background-color: %s; }",
        config->couleur_fond);

    gtk_css_provider_load_from_string(provider, css_data);

    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_provider(context,
                                   GTK_STYLE_PROVIDER(provider),
                                   GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);
}

void conteneur_initialiser(Conteneur *config) {
    if (!config) return;

    config->widget = NULL;
    config->orientation = CONTENEUR_VERTICAL; // Vertical par défaut
    config->espacement = 0;                   // Pas d'espace par défaut
    config->homogene = false;
    config->id_css = NULL;
    config->couleur_fond = NULL;

    config->marge_haut = 0;
    config->marge_bas = 0;
    config->marge_gauche = 0;
    config->marge_droite = 0;
}

GtkWidget* conteneur_creer(Conteneur *config) {
    if (!config) return NULL;

    // Conversion de notre enum vers l'enum GTK
    GtkOrientation or = (config->orientation == CONTENEUR_VERTICAL) ?
                        GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;

    // 1. Création
    config->widget = gtk_box_new(or, config->espacement);

    // 2. Propriétés
    gtk_box_set_homogeneous(GTK_BOX(config->widget), config->homogene);

    // 3. Marges
    gtk_widget_set_margin_top(config->widget, config->marge_haut);
    gtk_widget_set_margin_bottom(config->widget, config->marge_bas);
    gtk_widget_set_margin_start(config->widget, config->marge_gauche);
    gtk_widget_set_margin_end(config->widget, config->marge_droite);

    // 4. Identifiant CSS
    if (config->id_css) {
        gtk_widget_set_name(config->widget, config->id_css);
    }

    // 5. Style (Couleur de fond)
    _conteneur_appliquer_css(config->widget, config);

    return config->widget;
}

void conteneur_ajouter(Conteneur *config, GtkWidget *enfant) {
    if (!config || !config->widget || !enfant) return;

    // En GTK4, on utilise append pour ajouter à la fin
    gtk_box_append(GTK_BOX(config->widget), enfant);
}
