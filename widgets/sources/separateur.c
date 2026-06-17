#include "../headers/separateur.h"
#include <string.h>

void separateur_initialiser(Separateur *cfg)
{
    if (!cfg) return;
    memset(cfg, 0, sizeof(Separateur));
    cfg->orientation = SEPARATEUR_HORIZONTAL;
}

GtkWidget *separateur_creer(Separateur *cfg)
{
    GtkOrientation orient = (cfg->orientation == SEPARATEUR_HORIZONTAL)
        ? GTK_ORIENTATION_HORIZONTAL
        : GTK_ORIENTATION_VERTICAL;

    cfg->widget = gtk_separator_new(orient);

    if (cfg->marge_haut   > 0) gtk_widget_set_margin_top(cfg->widget,    cfg->marge_haut);
    if (cfg->marge_bas    > 0) gtk_widget_set_margin_bottom(cfg->widget,  cfg->marge_bas);
    if (cfg->marge_debut  > 0) gtk_widget_set_margin_start(cfg->widget,   cfg->marge_debut);
    if (cfg->marge_fin    > 0) gtk_widget_set_margin_end(cfg->widget,     cfg->marge_fin);

    return cfg->widget;
}

void separateur_set_marges(Separateur *cfg, int haut, int bas, int debut, int fin)
{
    cfg->marge_haut  = haut;
    cfg->marge_bas   = bas;
    cfg->marge_debut = debut;
    cfg->marge_fin   = fin;
    if (cfg->widget) {
        gtk_widget_set_margin_top(cfg->widget,    haut);
        gtk_widget_set_margin_bottom(cfg->widget,  bas);
        gtk_widget_set_margin_start(cfg->widget,   debut);
        gtk_widget_set_margin_end(cfg->widget,     fin);
    }
}
