#include "../headers/conteneur.h"
#include <stdio.h>

/* Convertisseur interne pour l'alignement GTK */
static GtkAlign _convertir_align(ConteneurAlignement a)
{
    switch (a)
    {
    case ALIGNEMENT_DEBUT:
        return GTK_ALIGN_START;
    case ALIGNEMENT_FIN:
        return GTK_ALIGN_END;
    case ALIGNEMENT_CENTRE:
        return GTK_ALIGN_CENTER;
    case ALIGNEMENT_REMPLIR:
    default:
        return GTK_ALIGN_FILL;
    }
}

/* Génération du CSS avancé */
static void _conteneur_appliquer_css(GtkWidget *widget, Conteneur *config)
{
    GtkCssProvider *provider = gtk_css_provider_new();
    char css_data[1024] = ""; // Buffer plus grand pour contenir tout le CSS
    char style_body[1024] = "";

    // 1. Construction du corps du style
    if (config->couleur_fond)
    {
        char buf[64];
        snprintf(buf, 64, "background-color: %s; ", config->couleur_fond);
        strcat(style_body, buf);
    }

    if (config->bordure_largeur > 0 && config->bordure_couleur)
    {
        char buf[128];
        snprintf(buf, 128, "border: %dpx solid %s; ", config->bordure_largeur, config->bordure_couleur);
        strcat(style_body, buf);
    }

    if (config->bordure_rayon > 0)
    {
        char buf[64];
        snprintf(buf, 64, "border-radius: %dpx; ", config->bordure_rayon);
        strcat(style_body, buf);
    }

    // Application du padding via CSS (plus fiable pour les GtkBox en GTK4 que les propriétés widget)
    if (config->padding.haut > 0 || config->padding.gauche > 0)
    {
        char buf[128];
        snprintf(buf, 128, "padding: %dpx %dpx %dpx %dpx; ",
                 config->padding.haut, config->padding.droite,
                 config->padding.bas, config->padding.gauche);
        strcat(style_body, buf);
    }

    // 2. Si on a du style à appliquer
    if (strlen(style_body) > 0)
    {
        // "box" cible le noeud GtkBox standard
        snprintf(css_data, sizeof(css_data), "box { %s }", style_body);

        gtk_css_provider_load_from_string(provider, css_data);
        GtkStyleContext *context = gtk_widget_get_style_context(widget);
        gtk_style_context_add_provider(context,
                                       GTK_STYLE_PROVIDER(provider),
                                       GTK_STYLE_PROVIDER_PRIORITY_USER);
    }

    g_object_unref(provider);
}

void conteneur_initialiser(Conteneur *config)
{
    if (!config)
        return;

    config->widget = NULL;
    config->orientation = CONTENEUR_VERTICAL;
    config->espacement = 0;
    config->homogene = false;

    // Dimensions par défaut (-1 = auto)
    config->taille.largeur = -1;
    config->taille.hauteur = -1;

    // Alignement par défaut (Remplir tout l'espace)
    config->align_x = ALIGNEMENT_REMPLIR;
    config->align_y = ALIGNEMENT_REMPLIR;

    // Expansion des enfants - FALSE par defaut (les enfants ne s'etendent pas)
    config->enfants_hexpand = false;
    config->enfants_vexpand = false;

    // Marges (Extérieures) à 0
    config->marges.haut = 0;
    config->marges.bas = 0;
    config->marges.gauche = 0;
    config->marges.droite = 0;

    // Padding (Intérieur) à 0
    config->padding.haut = 0;
    config->padding.bas = 0;
    config->padding.gauche = 0;
    config->padding.droite = 0;

    // Style
    config->id_css = NULL;
    config->couleur_fond = NULL;
    config->bordure_largeur = 0;
    config->bordure_couleur = "black";
    config->bordure_rayon = 0;
}

GtkWidget *conteneur_creer(Conteneur *config)
{
    if (!config)
        return NULL;

    GtkOrientation or = (config->orientation == CONTENEUR_VERTICAL) ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;

    // 1. Création
    config->widget = gtk_box_new(or, config->espacement);

    // 2. Propriétés de base
    gtk_box_set_homogeneous(GTK_BOX(config->widget), config->homogene);

    // 3. Dimensions (taille forcée)
    // -1 signifie "laisser GTK décider", sinon on impose la taille
    gtk_widget_set_size_request(config->widget, config->taille.largeur, config->taille.hauteur);

    // 4. Alignement
    gtk_widget_set_halign(config->widget, _convertir_align(config->align_x));
    gtk_widget_set_valign(config->widget, _convertir_align(config->align_y));

    // 5. Marges Extérieures
    gtk_widget_set_margin_top(config->widget, config->marges.haut);
    gtk_widget_set_margin_bottom(config->widget, config->marges.bas);
    gtk_widget_set_margin_start(config->widget, config->marges.gauche);
    gtk_widget_set_margin_end(config->widget, config->marges.droite);

    // 6. ID pour CSS
    if (config->id_css)
    {
        gtk_widget_set_name(config->widget, config->id_css);
    }

    // 7. Style Complet (Couleur, Bordure, Padding/Rembourrage)
    _conteneur_appliquer_css(config->widget, config);

    return config->widget;
}

void conteneur_ajouter(Conteneur *config, GtkWidget *enfant)
{
    if (!config || !config->widget || !enfant)
        return;

    gtk_box_append(GTK_BOX(config->widget), enfant);

    // Apply hexpand/vexpand settings to the child
    gtk_widget_set_hexpand(enfant, config->enfants_hexpand);
    gtk_widget_set_vexpand(enfant, config->enfants_vexpand);

    // Set alignment for proper distribution
    if (config->enfants_hexpand)
    {
        gtk_widget_set_halign(enfant, GTK_ALIGN_FILL);
    }
    if (config->enfants_vexpand)
    {
        gtk_widget_set_valign(enfant, GTK_ALIGN_FILL);
    }
}
