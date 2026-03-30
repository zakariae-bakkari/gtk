#include "../headers/conteneur.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    config->scroll_widget = NULL;
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
    config->bordure_couleur = malloc(strlen("black") + 1);
    strcpy(config->bordure_couleur, "black");
    config->bordure_rayon = 0;

    // Scrolling defaults
    config->scroll_mode = SCROLL_NONE;
    config->scroll_min_width = 0;
    config->scroll_min_height = 0;
    config->scroll_overlay = true;
}

GtkWidget *conteneur_creer(Conteneur *config)
{
    if (!config)
        return NULL;

    GtkOrientation or = (config->orientation == CONTENEUR_VERTICAL) ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;

    // 1. Création du conteneur principal (GtkBox)
    config->widget = gtk_box_new(or, config->espacement);

    // 2. Propriétés de base du conteneur
    gtk_box_set_homogeneous(GTK_BOX(config->widget), config->homogene);

    // 3. Configuration du scrolling si nécessaire
    GtkWidget *top_widget = config->widget; // Widget à retourner

    if (config->scroll_mode != SCROLL_NONE)
    {
        // Créer un GtkScrolledWindow
        config->scroll_widget = gtk_scrolled_window_new();

        // Configurer les politiques de défilement
        GtkPolicyType h_policy = GTK_POLICY_NEVER;
        GtkPolicyType v_policy = GTK_POLICY_NEVER;

        switch (config->scroll_mode)
        {
        case SCROLL_HORIZONTAL:
            h_policy = GTK_POLICY_AUTOMATIC;
            break;
        case SCROLL_VERTICAL:
            v_policy = GTK_POLICY_AUTOMATIC;
            break;
        case SCROLL_BOTH:
            h_policy = GTK_POLICY_AUTOMATIC;
            v_policy = GTK_POLICY_AUTOMATIC;
            break;
        case SCROLL_NONE:
        default:
            break;
        }

        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(config->scroll_widget),
                                       h_policy, v_policy);

        // Configurer les barres de défilement overlay
        gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(config->scroll_widget),
                                                  config->scroll_overlay);

        // Définir la taille minimale de la zone de défilement si spécifiée
        if (config->scroll_min_width > 0)
        {
            gtk_scrolled_window_set_min_content_width(
                GTK_SCROLLED_WINDOW(config->scroll_widget),
                config->scroll_min_width);
        }
        if (config->scroll_min_height > 0)
        {
            gtk_scrolled_window_set_min_content_height(
                GTK_SCROLLED_WINDOW(config->scroll_widget),
                config->scroll_min_height);
        }

        // Ajouter le conteneur dans la zone de défilement
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(config->scroll_widget), config->widget);
        // Le widget à retourner devient la ScrolledWindow
        top_widget = config->scroll_widget;
        // Appliquer les propriétés de dimension et alignement au widget scrollable
        if (config->taille.largeur != -1 || config->taille.hauteur != -1)
        { gtk_widget_set_size_request(top_widget, config->taille.largeur, config->taille.hauteur);}

        gtk_widget_set_halign(top_widget, _convertir_align(config->align_x));
        gtk_widget_set_valign(top_widget, _convertir_align(config->align_y));

        gtk_widget_set_margin_top(top_widget, config->marges.haut);
        gtk_widget_set_margin_bottom(top_widget, config->marges.bas);
        gtk_widget_set_margin_start(top_widget, config->marges.gauche);
        gtk_widget_set_margin_end(top_widget, config->marges.droite);

        if (config->id_css)
        { gtk_widget_set_name(top_widget, config->id_css); }
    }
    else
    {   // Pas de scrolling - configuration normale
        gtk_widget_set_size_request(config->widget, config->taille.largeur, config->taille.hauteur);
        gtk_widget_set_halign(config->widget, _convertir_align(config->align_x));
        gtk_widget_set_valign(config->widget, _convertir_align(config->align_y));

        gtk_widget_set_margin_top(config->widget, config->marges.haut);
        gtk_widget_set_margin_bottom(config->widget, config->marges.bas);
        gtk_widget_set_margin_start(config->widget, config->marges.gauche);
        gtk_widget_set_margin_end(config->widget, config->marges.droite);

        if (config->id_css)
        {
            gtk_widget_set_name(config->widget, config->id_css);
        }
    }
    // Style CSS (appliqué au conteneur principal, pas à la scrolled window)
    _conteneur_appliquer_css(config->widget, config);
    return top_widget;
}

void conteneur_ajouter(Conteneur *config, GtkWidget *enfant)
{
    if (!config || !config->widget || !enfant)
        return;

    gtk_box_append(GTK_BOX(config->widget), enfant);
}

// Scrolling configuration helper functions
void conteneur_set_scrollable(Conteneur *config, WidgetScrollMode mode)
{
    if (!config)
        return;
    config->scroll_mode = mode;
}

void conteneur_set_scroll_size(Conteneur *config, int min_width, int min_height)
{
    if (!config)
        return;
    config->scroll_min_width = min_width;
    config->scroll_min_height = min_height;
}

void conteneur_set_scroll_overlay(Conteneur *config, gboolean overlay)
{
    if (!config)
        return;
    config->scroll_overlay = overlay;
}
