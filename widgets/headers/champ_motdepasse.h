#ifndef CHAMP_MOTDEPASSE_H
#define CHAMP_MOTDEPASSE_H

#include <gtk/gtk.h>
#include <stdbool.h>
#include "common.h"

typedef struct
{
   int min_len;
   bool require_digit;
   bool require_upper;
   bool require_symbol;
} ChampPasswordPolicy;

typedef struct
{
   GtkWidget *widget; // GtkPasswordEntry
   char *id_css;      // ID CSS

   // Contenu / contraintes
   char *placeholder;
   int max_length; // 0 = illimité
   bool required;
   ChampPasswordPolicy policy;

   // Comportement
   bool reveal_toggle; // afficher/masquer bouton
   bool sensitive;     // actif/inactif

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
} ChampMotDePasse;

void champ_motdepasse_initialiser(ChampMotDePasse *cfg);
GtkWidget *champ_motdepasse_creer(ChampMotDePasse *cfg);

const char *champ_motdepasse_get_texte(ChampMotDePasse *cfg);
void champ_motdepasse_set_texte(ChampMotDePasse *cfg, const char *texte);
void champ_motdepasse_set_placeholder(ChampMotDePasse *cfg, const char *ph);
void champ_motdepasse_set_max_length(ChampMotDePasse *cfg, int max_len);
void champ_motdepasse_set_required(ChampMotDePasse *cfg, bool required);
void champ_motdepasse_set_reveal_toggle(ChampMotDePasse *cfg, bool reveal);
void champ_motdepasse_set_policy(ChampMotDePasse *cfg, ChampPasswordPolicy policy);
void champ_motdepasse_set_size(ChampMotDePasse *cfg, int width, int height);
void champ_motdepasse_free(ChampMotDePasse *cfg);

#endif // CHAMP_MOTDEPASSE_H
