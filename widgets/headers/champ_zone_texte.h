#ifndef CHAMP_ZONE_TEXTE_H
#define CHAMP_ZONE_TEXTE_H

#include <gtk/gtk.h>
#include "common.h"

typedef void (*ChampZoneOnChange)(GtkTextBuffer *buffer, gpointer user_data);

typedef struct
{
   GtkWidget *widget; // GtkTextView
   char *id_css;      // CSS id

   // Contenu & contraintes
   char *texte;        // valeur initiale
   int max_length;     // 0 = illimité
   gboolean wrap_word; // true = GTK_WRAP_WORD, false = GTK_WRAP_CHAR
   gboolean sensitive; // actif/inactif
   gboolean required;  // non vide requis

   // Taille du widget
   WidgetSize size;

   // Style (utilise la structure commune)
   WidgetStyle style;

   // Événements
   ChampZoneOnChange on_change;
   WidgetOnInvalid on_invalid;
   gpointer user_data;
} ChampZoneTexte;

// fonctions d'initialisation et gestion de la mémoire
void champ_zone_texte_initialiser(ChampZoneTexte *cfg);
GtkWidget *champ_zone_texte_creer(ChampZoneTexte *cfg);
void champ_zone_texte_free(ChampZoneTexte *cfg);

// fonctions d'accès et de modification (getters/setters)
char *champ_zone_texte_get_texte(ChampZoneTexte *cfg);
void champ_zone_texte_set_texte(ChampZoneTexte *cfg, const char *texte);
void champ_zone_texte_set_max_length(ChampZoneTexte *cfg, int max_len);
void champ_zone_texte_set_wrap_word(ChampZoneTexte *cfg, gboolean wrap_word);
void champ_zone_texte_set_sensitive(ChampZoneTexte *cfg, gboolean sensitive);
void champ_zone_texte_set_required(ChampZoneTexte *cfg, gboolean required);
void champ_zone_texte_set_size(ChampZoneTexte *cfg, int width, int height);

#endif // CHAMP_ZONE_TEXTE_H
