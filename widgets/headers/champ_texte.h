#ifndef CHAMP_TEXTE_H
#define CHAMP_TEXTE_H

#include <gtk/gtk.h>
#include "common.h"

/**
 * Type d'entrée pour le champ texte
 */
typedef enum
{
   CHAMP_TEXTE_TYPE_TEXT,   // Texte libre (défaut)
   CHAMP_TEXTE_TYPE_EMAIL,  // Adresse e-mail
   CHAMP_TEXTE_TYPE_URL,    // URL
   CHAMP_TEXTE_TYPE_SEARCH, // Champ de recherche
} ChampTexteType;

/**
 * Politique de validation pour le champ texte
 */
typedef struct
{
   int min_len;             // Longueur minimale (0 = aucune)
   int max_len;             // Longueur maximale (0 = illimitée)
   const char *pattern;     // Expression régulière de validation (NULL = aucune)
   gboolean no_whitespace;  // Interdit les espaces
   gboolean no_digits;      // Interdit les chiffres
   gboolean only_digits;    // Uniquement des chiffres (pour texte numérique simple)
} ChampTextePolicy;

typedef struct
{
   GtkWidget *widget; // GtkEntry
   char *id_css;      // ID CSS

   // Contenu / contraintes
   char *placeholder;
   int max_length;        // 0 = illimité (raccourci direct GTK)
   gboolean required;
   ChampTexteType type;
   ChampTextePolicy policy;

   // Comportement
   gboolean sensitive;    // actif/inactif
   gboolean editable;     // éditable ou lecture seule

   // Icônes dans le champ (optionnelles)
   const char *icon_primary;   // Nom d'icône GTK (ex: "edit-find-symbolic")
   const char *icon_secondary; // Nom d'icône GTK secondaire (ex: "edit-clear-symbolic")

   // Taille du widget
   WidgetSize size;

   // Style (utilise la structure commune)
   WidgetStyle style;

   // Événements
   WidgetOnChange on_change;
   WidgetOnActivate on_activate;
   WidgetOnInvalid on_invalid;
   gpointer user_data;
} ChampTexte;

/* Cycle de vie */
void champ_texte_initialiser(ChampTexte *cfg);
GtkWidget *champ_texte_creer(ChampTexte *cfg);
void champ_texte_free(ChampTexte *cfg);

/* Getters / Setters */
const char *champ_texte_get_texte(ChampTexte *cfg);
void champ_texte_set_texte(ChampTexte *cfg, const char *texte);
void champ_texte_set_placeholder(ChampTexte *cfg, const char *ph);
void champ_texte_set_max_length(ChampTexte *cfg, int max_len);
void champ_texte_set_required(ChampTexte *cfg, gboolean required);
void champ_texte_set_editable(ChampTexte *cfg, gboolean editable);
void champ_texte_set_type(ChampTexte *cfg, ChampTexteType type);
void champ_texte_set_policy(ChampTexte *cfg, ChampTextePolicy policy);
void champ_texte_set_size(ChampTexte *cfg, int width, int height);
void champ_texte_set_icons(ChampTexte *cfg, const char *icon_primary, const char *icon_secondary);

/* Validation manuelle */
gboolean champ_texte_valider(ChampTexte *cfg);

#endif // CHAMP_TEXTE_H