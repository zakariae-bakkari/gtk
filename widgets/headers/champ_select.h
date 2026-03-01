#ifndef CHAMP_SELECT_H
#define CHAMP_SELECT_H

#include <gtk/gtk.h>
#include <stdbool.h>
#include "common.h"

typedef void (*ChampSelectOnChange)(GtkDropDown *dd, gpointer user_data);
typedef void (*WidgetOnInvalid)(GtkWidget *widget, const char *message, gpointer user_data);

typedef struct
{
   GtkWidget *widget; // GtkDropDown
   char *id_css;      // ID CSS

   // Modèle de données
   GtkStringList *model; // liste de chaînes

   // État & contraintes
   int selected_index; // -1 = none
   bool required;      // sélection obligatoire
   bool enable_search; // futur (avec factory)

   // Taille du widget
   int width;  // largeur en pixels (0 = 100% largeur)
   int height; // hauteur en pixels (0 = auto)

   // Style (utilise la structure commune)
   WidgetStyle style;

   // Événements
   ChampSelectOnChange on_change;
   WidgetOnInvalid on_invalid;
   gpointer user_data;
} ChampSelect;

void champ_select_initialiser(ChampSelect *cfg);
GtkWidget *champ_select_creer(ChampSelect *cfg);

void champ_select_add_item(ChampSelect *cfg, const char *label);
void champ_select_set_items(ChampSelect *cfg, const char **labels, int count);

int champ_select_get_index(ChampSelect *cfg);
void champ_select_set_index(ChampSelect *cfg, int index);
const char *champ_select_get_string(ChampSelect *cfg);

void champ_select_set_size(ChampSelect *cfg, int width, int height);
void champ_select_free(ChampSelect *cfg);

#endif // CHAMP_SELECT_H
