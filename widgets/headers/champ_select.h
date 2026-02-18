#ifndef CHAMP_SELECT_H
#define CHAMP_SELECT_H

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
} ChampSelectStyle;

typedef void (*ChampSelectOnChange)(GtkDropDown *dd, gpointer user_data);
typedef void (*ChampOnInvalid)(GtkWidget *widget, const char *message, gpointer user_data);

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

   // Style
   ChampSelectStyle style;

   // Événements
   ChampSelectOnChange on_change;
   ChampOnInvalid on_invalid;
   gpointer user_data;
} ChampSelect;

void champ_select_initialiser(ChampSelect *cfg);
GtkWidget *champ_select_creer(ChampSelect *cfg);

void champ_select_add_item(ChampSelect *cfg, const char *label);
void champ_select_set_items(ChampSelect *cfg, const char **labels, int count);

int champ_select_get_index(ChampSelect *cfg);
void champ_select_set_index(ChampSelect *cfg, int index);
const char *champ_select_get_string(ChampSelect *cfg);

#endif // CHAMP_SELECT_H
