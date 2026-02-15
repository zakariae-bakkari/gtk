#include "../headers/bouton.h"
#include <stdio.h>
#include <string.h>

static void _bouton_appliquer_css(Bouton *config)
{
    if (!config->id_css || !config->widget)
        return;

    GtkCssProvider *provider = gtk_css_provider_new();
    char css_buffer[2048] = "";

    // En GTK4, on utilise une syntaxe CSS plus robuste avec !important
    snprintf(css_buffer, sizeof(css_buffer),
             "#%s { "
             "background-color: %s !important; "
             "color: %s !important; "
             "border-radius: %dpx; "
             "border: %dpx solid %s; "
             "padding: 10px 20px; "
             "font-weight: %s; "
             "} "
             "#%s:hover { "
             "background-color: %s !important; "
             "color: %s !important; "
             "}",
             config->id_css,
             config->style.bg_normal ? config->style.bg_normal : "#3498db",
             config->style.fg_normal ? config->style.fg_normal : "black",
             config->style.rayon_arrondi,
             config->style.epaisseur_bordure,
             config->style.couleur_bordure ? config->style.couleur_bordure : "transparent",
             config->style.gras ? "bold" : "normal",
             config->id_css,
             config->style.bg_hover ? config->style.bg_hover : "#2980b9",
             config->style.fg_hover ? config->style.fg_hover : "white");

    gtk_css_provider_load_from_string(provider, css_buffer);

    // Application avec la priorité maximale en GTK4
    gtk_style_context_add_provider(
        gtk_widget_get_style_context(config->widget),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref(provider);
}

void bouton_initialiser(Bouton *config)
{
    if (!config)
        return;
    memset(config, 0, sizeof(Bouton));
    config->id_css = "btn_defaut";
    config->texte = "Bouton";
    config->pos_icone = ICONE_GAUCHE;
    config->espacement_icone = 8;

    // Initialiser le mode de taille à AUTO par défaut
    config->taille.mode = TAILLE_AUTO;
    config->taille.largeur = -1;
    config->taille.hauteur = -1;
    config->taille.largeur_min = -1;
    config->taille.hauteur_min = -1;

    config->style.bg_normal = "#3498db";
    config->style.fg_normal = "white";
    config->style.rayon_arrondi = 5;
    config->curseur = CURSEUR_MAIN;
    config->est_actif = true;
}

GtkWidget *bouton_creer(Bouton *config)
{
    if (!config)
        return NULL;

    config->widget = gtk_button_new();

    // Important : On définit le nom CSS avant d'appliquer le style
    gtk_widget_set_name(config->widget, config->id_css);

    GtkWidget *box = gtk_box_new((config->pos_icone == ICONE_HAUT || config->pos_icone == ICONE_BAS) ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL,
                                 config->espacement_icone);

    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    if (config->nom_icone)
    {
        GtkWidget *img = gtk_image_new_from_icon_name(config->nom_icone);
        if (config->pos_icone == ICONE_DROITE || config->pos_icone == ICONE_BAS)
        {
            gtk_box_append(GTK_BOX(box), gtk_label_new(config->texte));
            gtk_box_append(GTK_BOX(box), img);
        }
        else
        {
            gtk_box_append(GTK_BOX(box), img);
            if (config->pos_icone != ICONE_SEULE)
                gtk_box_append(GTK_BOX(box), gtk_label_new(config->texte));
        }
    }
    else
    {
        gtk_box_append(GTK_BOX(box), gtk_label_new(config->texte));
    }

    gtk_button_set_child(GTK_BUTTON(config->widget), box);

    /* --- Appliquer le mode de dimensionnement --- */
    switch (config->taille.mode)
    {
    case TAILLE_AUTO:
        // Laisser le bouton se dimensionner automatiquement (défaut GTK)
        // Les parametres largeur/hauteur sont ignores
        gtk_widget_set_size_request(config->widget, -1, -1);
        // NO hexpand for AUTO mode - button stays compact
        gtk_widget_set_hexpand(config->widget, FALSE);
        break;

    case TAILLE_FIXE:
        // Dimensions exactes specifiees
        if (config->taille.largeur > 0 && config->taille.hauteur > 0)
        {
            gtk_widget_set_size_request(config->widget,
                                        config->taille.largeur,
                                        config->taille.hauteur);
        }
        else if (config->taille.largeur > 0)
        {
            gtk_widget_set_size_request(config->widget,
                                        config->taille.largeur,
                                        -1);
        }
        else if (config->taille.hauteur > 0)
        {
            gtk_widget_set_size_request(config->widget,
                                        -1,
                                        config->taille.hauteur);
        }

        // For FIXED mode with large width (>300px), expand to fill
        if (config->taille.largeur > 300)
        {
            gtk_widget_set_hexpand(config->widget, TRUE);
            gtk_widget_set_halign(config->widget, GTK_ALIGN_FILL);
        }
        else
        {
            gtk_widget_set_hexpand(config->widget, FALSE);
        }
        break;

    case TAILLE_FIT_CONTENT:
        // Ajuster au contenu avec dimensions minimales optionnelles
        gtk_widget_set_size_request(config->widget,
                                    config->taille.largeur_min > 0 ? config->taille.largeur_min : -1,
                                    config->taille.hauteur_min > 0 ? config->taille.hauteur_min : -1);
        // FIT_CONTENT doesn't expand
        gtk_widget_set_hexpand(config->widget, FALSE);
        break;

    default:
        gtk_widget_set_size_request(config->widget, -1, -1);
        gtk_widget_set_hexpand(config->widget, FALSE);
        break;
    }

    if (config->tooltip)
        gtk_widget_set_tooltip_text(config->widget, config->tooltip);
    if (config->on_clic)
        g_signal_connect(config->widget, "clicked", G_CALLBACK(config->on_clic), config->user_data);

    _bouton_appliquer_css(config);

    return config->widget;
}

void bouton_set_texte(Bouton *config, const char *nouveau_texte)
{
    if (!config || !config->widget || !nouveau_texte)
        return;
    config->texte = (char *)nouveau_texte;
    // La mise à jour du texte nécessiterait de recréer le contenu du bouton
}

/**
 * Change la taille du bouton dynamiquement après sa création
 * @param config : Structure du bouton
 * @param mode : Mode de dimensionnement (AUTO, FIXE, FIT_CONTENT)
 * @param largeur : Largeur en pixels (ignorée en mode AUTO)
 * @param hauteur : Hauteur en pixels (ignorée en mode AUTO)
 */
void bouton_set_taille(Bouton *config, BoutonTailleMode mode, int largeur, int hauteur)
{
    if (!config || !config->widget)
        return;

    config->taille.mode = mode;
    config->taille.largeur = largeur;
    config->taille.hauteur = hauteur;

    switch (mode)
    {
    case TAILLE_AUTO:
        gtk_widget_set_size_request(config->widget, -1, -1);
        break;

    case TAILLE_FIXE:
        gtk_widget_set_size_request(config->widget, largeur, hauteur);
        break;

    case TAILLE_FIT_CONTENT:
        gtk_widget_set_size_request(config->widget,
                                    largeur > 0 ? largeur : -1,
                                    hauteur > 0 ? hauteur : -1);
        break;

    default:
        gtk_widget_set_size_request(config->widget, -1, -1);
        break;
    }
}

/**
 * Change uniquement la largeur du bouton
 * Utile pour les boutons avec hauteur automatique
 * @param config : Structure du bouton
 * @param largeur : Largeur en pixels
 */
void bouton_set_largeur(Bouton *config, int largeur)
{
    if (!config || !config->widget)
        return;
    if (config->taille.mode == TAILLE_AUTO)
    {
        config->taille.mode = TAILLE_FIXE;
    }
    config->taille.largeur = largeur;
    gtk_widget_set_size_request(config->widget, largeur, config->taille.hauteur);
}

/**
 * Change uniquement la hauteur du bouton
 * Utile pour les boutons avec largeur automatique
 * @param config : Structure du bouton
 * @param hauteur : Hauteur en pixels
 */
void bouton_set_hauteur(Bouton *config, int hauteur)
{
    if (!config || !config->widget)
        return;
    if (config->taille.mode == TAILLE_AUTO)
    {
        config->taille.mode = TAILLE_FIXE;
    }
    config->taille.hauteur = hauteur;
    gtk_widget_set_size_request(config->widget, config->taille.largeur, hauteur);
}
