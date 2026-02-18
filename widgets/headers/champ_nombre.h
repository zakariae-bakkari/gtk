#ifndef CHAMP_NOMBRE_H
#define CHAMP_NOMBRE_H

#include <gtk/gtk.h>
#include <stdbool.h>

typedef struct
{
   char *bg_normal;
   char *bg_focus;
   char *fg_normal;
   int epaisseur_bordure;
   char *couleur_bordure;
   int rayon_arrondi;
   bool gras;
   bool italique;
   int taille_texte_px; // 0 = défaut
} ChampNombreStyle;

typedef void (*ChampNombreOnChange)(GtkSpinButton *spin, gpointer user_data);
typedef void (*ChampOnInvalid)(GtkWidget *widget, const char *message, gpointer user_data);

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

   // Style
   ChampNombreStyle style;

   // Événements
   ChampNombreOnChange on_change;
   ChampOnInvalid on_invalid;
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

#endif // CHAMP_NOMBRE_H
