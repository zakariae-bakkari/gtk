#ifndef CHAMP_TEXTE_H
#define CHAMP_TEXTE_H

#include <gtk/gtk.h>
#include <stdbool.h>

// Style configurable pour ChampTexte (GtkEntry)
typedef struct
{
   char *bg_normal;
   char *bg_focus;
   char *fg_normal;
   char *placeholder_color;
   int epaisseur_bordure;
   char *couleur_bordure;
   int rayon_arrondi;
   bool gras;
   bool italique;
   int taille_texte_px; // 0 = défaut
} ChampTexteStyle;

// Callbacks
typedef void (*ChampOnChange)(GtkEditable *editable, gpointer user_data);
typedef void (*ChampOnActivate)(GtkEntry *entry, gpointer user_data);
typedef void (*ChampOnInvalid)(GtkWidget *widget, const char *message, gpointer user_data);

typedef struct
{
   GtkWidget *widget; // GtkEntry
   char *id_css;      // ID CSS

   // Contenu
   char *texte;       // valeur initiale
   char *placeholder; // aide visuelle

   // Contraintes / validation
   int max_length;      // 0 = illimité
   bool required;       // valeur non vide requise
   char *regex_pattern; // validation optionnelle (POSIX/GRegex)

   // Etat & comportement
   bool read_only; // non éditable (editable=false)
   bool sensitive; // actif/inactif

   // Style
   ChampTexteStyle style;

   // Événements
   ChampOnChange on_change;
   ChampOnActivate on_activate;
   ChampOnInvalid on_invalid;
   gpointer user_data;
} ChampTexte;

// API
void champ_texte_initialiser(ChampTexte *cfg);
GtkWidget *champ_texte_creer(ChampTexte *cfg);

// Getters/Setters
const char *champ_texte_get_texte(ChampTexte *cfg);
void champ_texte_set_texte(ChampTexte *cfg, const char *texte);
void champ_texte_set_placeholder(ChampTexte *cfg, const char *ph);
void champ_texte_set_max_length(ChampTexte *cfg, int max_len);
void champ_texte_set_read_only(ChampTexte *cfg, bool ro);
void champ_texte_set_required(ChampTexte *cfg, bool required);
void champ_texte_set_regex(ChampTexte *cfg, const char *pattern);

#endif // CHAMP_TEXTE_H
