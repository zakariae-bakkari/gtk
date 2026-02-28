#ifndef CHAMP_MOTDEPASSE_H
#define CHAMP_MOTDEPASSE_H

#include <gtk/gtk.h>
#include <stdbool.h>

typedef struct
{
   char *bg_normal;
   char *fg_normal;
   int epaisseur_bordure;
   char *couleur_bordure;
   int rayon_arrondi;
   bool gras;
   bool italique;
   int taille_texte_px; // 0 = défaut
} ChampMotDePasseStyle;

typedef struct
{
   int min_len;
   bool require_digit;
   bool require_upper;
   bool require_symbol;
} ChampPasswordPolicy;

typedef void (*ChampOnChange)(GtkEditable *editable, gpointer user_data);
typedef void (*ChampOnActivate)(GtkEntry *entry, gpointer user_data);
typedef void (*ChampOnInvalid)(GtkWidget *widget, const char *message, gpointer user_data);

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

   // Style
   ChampMotDePasseStyle style;

   // Événements
   ChampOnChange on_change;
   ChampOnActivate on_activate;
   ChampOnInvalid on_invalid;
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

#endif // CHAMP_MOTDEPASSE_H
