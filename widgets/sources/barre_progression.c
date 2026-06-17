#include "../headers/barre_progression.h"
#include <string.h>

void barre_progression_initialiser(BarreProgression *cfg)
{
    if (!cfg) return;
    memset(cfg, 0, sizeof(BarreProgression));
    cfg->fraction      = 1.0;
    cfg->afficher_texte = FALSE;
    cfg->sensitive     = TRUE;
    cfg->halign        = BARRE_ALIGN_CENTRE;
}

GtkWidget *barre_progression_creer(BarreProgression *cfg)
{
    cfg->widget = gtk_progress_bar_new();

    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(cfg->widget), cfg->fraction);
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(cfg->widget), cfg->afficher_texte);
    gtk_widget_set_sensitive(cfg->widget, cfg->sensitive);

    /* Alignement horizontal */
    GtkAlign gtk_align = GTK_ALIGN_CENTER;
    switch (cfg->halign) {
        case BARRE_ALIGN_DEBUT:    gtk_align = GTK_ALIGN_START;  break;
        case BARRE_ALIGN_FIN:      gtk_align = GTK_ALIGN_END;    break;
        case BARRE_ALIGN_REMPLIR:  gtk_align = GTK_ALIGN_FILL;   break;
        default:                   gtk_align = GTK_ALIGN_CENTER; break;
    }
    gtk_widget_set_halign(cfg->widget, gtk_align);

    if (cfg->id_css)
        gtk_widget_add_css_class(cfg->widget, cfg->id_css);

    if (cfg->size.width > 0 || cfg->size.height > 0)
        gtk_widget_set_size_request(cfg->widget, cfg->size.width, cfg->size.height);

    return cfg->widget;
}

void barre_progression_set_fraction(BarreProgression *cfg, double fraction)
{
    if (fraction < 0.0) fraction = 0.0;
    if (fraction > 1.0) fraction = 1.0;
    cfg->fraction = fraction;
    if (cfg->widget)
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(cfg->widget), fraction);
}

double barre_progression_get_fraction(BarreProgression *cfg)
{
    return cfg ? cfg->fraction : 0.0;
}

void barre_progression_set_texte(BarreProgression *cfg, const char *texte)
{
    if (cfg && cfg->widget)
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(cfg->widget), texte);
}

void barre_progression_set_size(BarreProgression *cfg, int largeur, int hauteur)
{
    if (!cfg) return;
    cfg->size.width  = largeur;
    cfg->size.height = hauteur;
    if (cfg->widget)
        gtk_widget_set_size_request(cfg->widget, largeur, hauteur);
}

void barre_progression_add_css_class(BarreProgression *cfg, const char *classe)
{
    if (cfg && cfg->widget)
        gtk_widget_add_css_class(cfg->widget, classe);
}

void barre_progression_remove_css_class(BarreProgression *cfg, const char *classe)
{
    if (cfg && cfg->widget)
        gtk_widget_remove_css_class(cfg->widget, classe);
}
