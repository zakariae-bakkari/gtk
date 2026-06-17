#ifndef BARRE_PROGRESSION_H
#define BARRE_PROGRESSION_H

#include <gtk/gtk.h>
#include "common.h"

typedef enum {
    BARRE_ALIGN_DEBUT,
    BARRE_ALIGN_CENTRE,
    BARRE_ALIGN_FIN,
    BARRE_ALIGN_REMPLIR
} BarreAlign;

typedef struct {
    GtkWidget  *widget;    /* GtkProgressBar interne */
    char       *id_css;    /* classe CSS à ajouter   */
    double      fraction;  /* 0.0 – 1.0              */
    gboolean    afficher_texte;
    gboolean    sensitive;
    BarreAlign  halign;
    WidgetSize  size;
} BarreProgression;

void       barre_progression_initialiser(BarreProgression *cfg);
GtkWidget *barre_progression_creer(BarreProgression *cfg);
void       barre_progression_set_fraction(BarreProgression *cfg, double fraction);
double     barre_progression_get_fraction(BarreProgression *cfg);
void       barre_progression_set_texte(BarreProgression *cfg, const char *texte);
void       barre_progression_set_size(BarreProgression *cfg, int largeur, int hauteur);
void       barre_progression_add_css_class(BarreProgression *cfg, const char *classe);
void       barre_progression_remove_css_class(BarreProgression *cfg, const char *classe);

#endif // BARRE_PROGRESSION_H
