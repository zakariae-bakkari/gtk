#ifndef CHAMP_ZONE_TEXTE_H
#define CHAMP_ZONE_TEXTE_H

#include <gtk/gtk.h>
#include <stdbool.h>
#include "common.h"

typedef void (*ChampZoneOnChange)(GtkTextBuffer *buffer, gpointer user_data);

typedef struct
{
   GtkWidget *widget; // GtkTextView
   char *id_css;      // CSS id

   // Contenu & contraintes
   char *texte;    // valeur initiale
   int max_length; // 0 = illimité
   bool wrap_word; // true = GTK_WRAP_WORD, false = GTK_WRAP_CHAR
   bool sensitive; // actif/inactif
   bool required;  // non vide requis

   // Style (utilise la structure commune)
   WidgetStyle style;

   // Événements
   ChampZoneOnChange on_change;
   WidgetOnInvalid on_invalid;
   gpointer user_data;
} ChampZoneTexte;

void champ_zone_texte_initialiser(ChampZoneTexte *cfg);
GtkWidget *champ_zone_texte_creer(ChampZoneTexte *cfg);

char *champ_zone_texte_get_texte(ChampZoneTexte *cfg);
void champ_zone_texte_set_texte(ChampZoneTexte *cfg, const char *texte);
void champ_zone_texte_set_max_length(ChampZoneTexte *cfg, int max_len);
void champ_zone_texte_set_wrap_word(ChampZoneTexte *cfg, bool wrap_word);
void champ_zone_texte_set_sensitive(ChampZoneTexte *cfg, bool sensitive);
void champ_zone_texte_set_required(ChampZoneTexte *cfg, bool required);

#endif // CHAMP_ZONE_TEXTE_H
