#ifndef CHAMP_NOMBRE_H
#define CHAMP_NOMBRE_H

#include <gtk/gtk.h>
#include <stdbool.h>
#include "common.h"

typedef struct
{
   GtkWidget *widget; // GtkSpinButton
   char *id_css;      // ID CSS

   // Bornes et valeur
   double min;
   double max;
   double step;
   guint digits;  // nb décimales
   gboolean wrap; // boucle
   double valeur; // valeur initiale

   // Contraintes
   gboolean required; // si true, peut imposer min<=val<=max (déjà assuré)

   // Taille du widget
   int width;  // largeur en pixels (0 = 100% largeur)
   int height; // hauteur en pixels (0 = auto)

   // Style (utilise la structure commune)
   WidgetStyle style;

   // Événements
   WidgetOnChange on_change;
   WidgetOnActivate on_activate;
   WidgetOnInvalid on_invalid;
   gpointer user_data;
} ChampNombre;

void champ_nombre_initialiser(ChampNombre *cfg);
GtkWidget *champ_nombre_creer(ChampNombre *cfg);

double champ_nombre_get_valeur(ChampNombre *cfg);
void champ_nombre_set_valeur(ChampNombre *cfg, double v);
void champ_nombre_set_bornes(ChampNombre *cfg, double min, double max);
void champ_nombre_set_step(ChampNombre *cfg, double step);
void champ_nombre_set_digits(ChampNombre *cfg, guint digits);
void champ_nombre_set_wrap(ChampNombre *cfg, gboolean wrap);
void champ_nombre_set_size(ChampNombre *cfg, int width, int height);

// Style functions
void champ_nombre_set_style(ChampNombre *cfg, const WidgetStyle *style);
void champ_nombre_apply_style(ChampNombre *cfg);

// Callback functions
void champ_nombre_set_callbacks(ChampNombre *cfg, WidgetOnChange on_change, WidgetOnActivate on_activate, WidgetOnInvalid on_invalid, gpointer user_data);

void champ_nombre_free(ChampNombre *cfg);

#endif // CHAMP_NOMBRE_H
