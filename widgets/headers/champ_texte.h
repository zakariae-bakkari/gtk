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
   int min_len;            // Longueur minimale (0 = aucune)
   int max_len;            // Longueur maximale (0 = illimitée)
   const char *pattern;    // Expression régulière (NULL = aucune)
   gboolean no_whitespace; // Interdit les espaces
   gboolean no_digits;     // Interdit les chiffres
   gboolean only_digits;   // Uniquement des chiffres
} ChampTextePolicy;

typedef struct
{
   /**
    * IMPORTANT : c'est `container` qu'il faut ajouter au widget parent,
    * pas `widget`. `container` est un GtkBox vertical qui empile :
    *   1. cfg->widget      (GtkEntry)
    *   2. cfg->label_erreur (GtkLabel, caché quand tout est valide)
    */
   GtkWidget *container;    // GtkBox vertical — à passer à conteneur_ajouter()
   GtkWidget *widget;       // GtkEntry — pour lire/écrire la valeur
   GtkWidget *label_erreur; // GtkLabel du message d'erreur

   char *id_css; // ID CSS appliqué au GtkEntry

   // Contenu / contraintes
   char *placeholder;
   int max_length; // 0 = illimité
   gboolean required;
   ChampTexteType type;
   ChampTextePolicy policy;

   // Comportement
   gboolean sensitive; // actif/inactif
   gboolean editable;  // éditable ou lecture seule

   // Icônes dans le champ (optionnelles)
   const char *icon_primary;   // ex: "edit-find-symbolic"
   const char *icon_secondary; // ex: "edit-clear-symbolic"

   // Taille du GtkEntry (le container suit automatiquement)
   WidgetSize size;

   // Style de l'entrée
   WidgetStyle style;

   // Style du label d'erreur (laisser à NULL/0 pour les valeurs par défaut)
   gboolean show_error_label; // Afficher le label d'erreur sous le champ (défaut : TRUE)
   char *erreur_couleur;      // Couleur du texte d'erreur  (défaut : "#e74c3c")
   int erreur_taille_px;      // Taille de police en px      (défaut : 11px)

   // Événements
   WidgetOnChange on_change;
   WidgetOnActivate on_activate;
   WidgetOnInvalid on_invalid;
   gpointer user_data;
} ChampTexte;

/* Cycle de vie */
void champ_texte_initialiser(ChampTexte *cfg);
GtkWidget *champ_texte_creer(ChampTexte *cfg); // Retourne cfg->container
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

/* Validation manuelle — retourne TRUE si valide */
gboolean champ_texte_valider(ChampTexte *cfg);

#endif // CHAMP_TEXTE_H