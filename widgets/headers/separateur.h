#ifndef SEPARATEUR_H
#define SEPARATEUR_H

#include <gtk/gtk.h>

typedef enum {
    SEPARATEUR_HORIZONTAL,
    SEPARATEUR_VERTICAL
} SeparateurOrientation;

typedef struct {
    GtkWidget           *widget;
    SeparateurOrientation orientation;
    int                  marge_haut;
    int                  marge_bas;
    int                  marge_debut;
    int                  marge_fin;
} Separateur;

void       separateur_initialiser(Separateur *cfg);
GtkWidget *separateur_creer(Separateur *cfg);
void       separateur_set_marges(Separateur *cfg, int haut, int bas, int debut, int fin);

#endif // SEPARATEUR_H
