#ifndef CHAMP_SELECT_H
#define CHAMP_SELECT_H

#include "common.h"

typedef void (*ChampSelectOnChange)(Widget widget, void *user_data);

typedef struct
{
   Widget widget; // GtkDropDown
   char *id_css;      // ID CSS

   // Modèle de données
   void *model; // GtkStringList* list

   // État & contraintes
   int selected_index;     // -1 = none
   bool required;      // sélection obligatoire
   bool enable_search; // futur (avec factory)

   // Taille du widget
   WidgetSize size;

   // Style (utilise la structure commune)
   WidgetStyle style;

   // Événements
   ChampSelectOnChange on_change;
   WidgetOnInvalid on_invalid;
   void *user_data;
} ChampSelect;

void champ_select_initialiser(ChampSelect *cfg);
Widget champ_select_creer(ChampSelect *cfg);

void champ_select_add_item(ChampSelect *cfg, const char *label);
void champ_select_set_items(ChampSelect *cfg, const char **labels, int count);

int champ_select_get_index(ChampSelect *cfg);
void champ_select_set_index(ChampSelect *cfg, int index);
const char *champ_select_get_string(ChampSelect *cfg);

void champ_select_set_size(ChampSelect *cfg, int width, int height);
void champ_select_free(ChampSelect *cfg);

#endif // CHAMP_SELECT_H
